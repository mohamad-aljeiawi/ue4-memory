#ifndef TOUCH_H
#define TOUCH_H

#include <time.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/uinput.h>

#include <list>
#include <vector>
#include <chrono>
#include <thread>
#include <memory>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <climits>
#include <cerrno>
#include <linux/input.h>

template <class item>
class Channel
{
private:
    std::list<item> queue;
    std::mutex m;
    std::condition_variable cv;
    std::atomic<bool> closed{false};

public:
    Channel() = default;

    Channel(const Channel &) = delete;
    Channel &operator=(const Channel &) = delete;

    void close()
    {
        std::unique_lock<std::mutex> lock(m);
        closed = true;
        cv.notify_all();
    }

    bool is_closed() const
    {
        return closed.load();
    }

    bool put(const item &i)
    {
        std::unique_lock<std::mutex> lock(m);
        if (closed)
            return false;
        queue.push_back(i);
        cv.notify_one();
        return true;
    }

    bool get(item &out, bool wait = true)
    {
        std::unique_lock<std::mutex> lock(m);
        if (wait)
            cv.wait(lock, [&]()
                    { return closed || !queue.empty(); });

        if (queue.empty())
            return false;

        out = queue.front();
        queue.pop_front();
        return true;
    }
};

namespace TouchInput
{
#ifndef ABS_MAX
#define ABS_MAX 0x3f
#endif

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

    struct input_absinfo absX;
    struct input_absinfo absY;
    struct input_absinfo absSlot;
    const char letterBytes[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

    std::unique_ptr<char[]> randString(int n)
    {
        if (n <= 0)
            return nullptr;

        auto b = std::make_unique<char[]>(n + 1);

        for (int i = 0; i < n; i++)
        {
            b[i] = letterBytes[rand() % (sizeof(letterBytes) - 1)];
        }
        b[n] = '\0';

        return b;
    }

    int randNum(int n)
    {
        return (n > 0) ? rand() % n : 0;
    }

    int createUInput(int ifd)
    {
        const char *uinput = "/dev/uinput";

        if (ioctl(ifd, EVIOCGABS(ABS_MT_SLOT), &absSlot) < 0 ||
            ioctl(ifd, EVIOCGABS(ABS_MT_POSITION_X), &absX) < 0 ||
            ioctl(ifd, EVIOCGABS(ABS_MT_POSITION_Y), &absY) < 0)
        {
            perror("Failed to get abs info");
            return -1;
        }

        int ufd = open(uinput, O_WRONLY | O_NONBLOCK);
        if (ufd < 0)
        {
            perror("Unable to open uinput");
            return -1;
        }

        if (ioctl(ufd, UI_SET_EVBIT, EV_KEY) < 0 ||
            ioctl(ufd, UI_SET_KEYBIT, BTN_TOUCH) < 0 ||
            ioctl(ufd, UI_SET_EVBIT, EV_ABS) < 0 ||
            ioctl(ufd, UI_SET_ABSBIT, ABS_MT_POSITION_X) < 0 ||
            ioctl(ufd, UI_SET_ABSBIT, ABS_MT_POSITION_Y) < 0 ||
            ioctl(ufd, UI_SET_ABSBIT, ABS_MT_TRACKING_ID) < 0 ||
            ioctl(ufd, UI_SET_PROPBIT, INPUT_PROP_DIRECT) < 0)
        {
            perror("Failed to set uinput properties");
            close(ufd);
            return -1;
        }

        struct uinput_user_dev uidev;
        memset(&uidev, 0, sizeof(uidev));
        uidev.id.bustype = 0x1C;
        uidev.id.vendor = randNum(0x2000);
        uidev.id.product = randNum(0x2000);
        uidev.id.version = randNum(0x200);

        auto devName = randString(7);
        if (devName)
        {
            strncpy(uidev.name, devName.get(), UINPUT_MAX_NAME_SIZE - 1);
            uidev.name[UINPUT_MAX_NAME_SIZE - 1] = '\0';
        }

        uidev.absmin[ABS_MT_POSITION_X] = absX.minimum;
        uidev.absmax[ABS_MT_POSITION_X] = absX.maximum;
        uidev.absmin[ABS_MT_POSITION_Y] = absY.minimum;
        uidev.absmax[ABS_MT_POSITION_Y] = absY.maximum;
        uidev.absmin[ABS_MT_TRACKING_ID] = 0;
        uidev.absmax[ABS_MT_TRACKING_ID] = absSlot.maximum;

        if (write(ufd, &uidev, sizeof(uidev)) != sizeof(uidev) ||
            ioctl(ufd, UI_DEV_CREATE) < 0)
        {
            perror("Failed to create uinput device");
            close(ufd);
            return -1;
        }

        if (ioctl(ifd, EVIOCGRAB, 1) < 0)
        {
            perror("Warning: Failed to grab input device");
        }

        return ufd;
    }

    struct TouchContact
    {
        std::atomic<int> posX{-1};
        std::atomic<int> posY{-1};
        std::atomic<bool> enabled{false};
        std::atomic<int> displayX{-1};
        std::atomic<int> displayY{-1};
    };
    struct TouchRect
    {
        int x, y, width, height;

        TouchRect(int x = 0, int y = 0, int w = 0, int h = 0)
            : x(x), y(y), width(w), height(h) {}

        bool contains(int px, int py) const
        {
            return px >= x && px < (x + width) &&
                   py >= y && py < (y + height);
        }

        static TouchRect fromCenter(int centerX, int centerY, int radius)
        {
            return TouchRect(centerX - radius, centerY - radius, centerX + radius, centerY + radius);
        }

        static TouchRect fromCenter(int centerX, int centerY, int radiusX, int radiusY)
        {
            return TouchRect(centerX - radiusX, centerY - radiusY, centerX + radiusX, centerY + radiusY);
        }
    };

#define FAKE_CONTACT 9

    class TouchInputManager
    {
    private:
        int touchXMin = 0;
        int touchXMax = 0;
        int touchYMin = 0;
        int touchYMax = 0;
        int maxContacts = 0;
        std::atomic<int> displayWidth{0};
        std::atomic<int> displayHeight{0};
        std::atomic<int> displayRotation{0};

        int touchDeviceFd = -1;
        int uInputTouchFd = -1;

        std::atomic<bool> isBtnDown{false};
        std::atomic<bool> touchSend{false};
        std::atomic<bool> touchStart{false};
        std::atomic<bool> shouldStop{false};

        std::mutex touchSynMtx;
        std::mutex zoneCheckMutex;
        Channel<bool> touchChannel;
        std::unique_ptr<TouchContact[]> contacts;

        std::thread readerThread;
        std::thread writerThread;

    public:
        TouchInputManager() = default;
        ~TouchInputManager() { stop(); }

        TouchInputManager(const TouchInputManager &) = delete;
        TouchInputManager &operator=(const TouchInputManager &) = delete;

        void setDisplayInfo(int width, int height, int rotation)
        {
            displayWidth = width;
            displayHeight = height;
            displayRotation = rotation;
        }

        bool start()
        {
            if (touchStart.load())
                return true;

            touchDeviceFd = getTouchDevice();
            if (touchDeviceFd < 0)
            {
                perror("Unable to find touch device");
                return false;
            }

            uInputTouchFd = createUInput(touchDeviceFd);
            if (uInputTouchFd < 0)
            {
                perror("Unable to create virtual touch device");
                close(touchDeviceFd);
                touchDeviceFd = -1;
                return false;
            }

            touchXMin = absX.minimum;
            touchXMax = absX.maximum - absX.minimum + 1;
            touchYMin = absY.minimum;
            touchYMax = absY.maximum - absY.minimum + 1;
            maxContacts = absSlot.maximum + 1;

            contacts = std::make_unique<TouchContact[]>(maxContacts);

            shouldStop = false;
            readerThread = std::thread(&TouchInputManager::eventReaderThread, this);
            writerThread = std::thread(&TouchInputManager::eventWriterThread, this);

            touchStart = true;
            return true;
        }

        void stop()
        {
            if (!touchStart.load())
                return;

            shouldStop = true;
            touchChannel.close();

            if (readerThread.joinable())
                readerThread.join();
            if (writerThread.joinable())
                writerThread.join();

            if (touchDeviceFd >= 0)
            {
                close(touchDeviceFd);
                touchDeviceFd = -1;
            }

            if (uInputTouchFd >= 0)
            {
                close(uInputTouchFd);
                uInputTouchFd = -1;
            }

            contacts.reset();
            touchStart = false;
        }

        void sendTouchMove(int x, int y)
        {
            if (!touchStart.load() || !contacts)
                return;

            if (!touchSend.load())
            {
                touchSend = true;
            }

            int width = displayWidth.load();
            int height = displayHeight.load();
            int rotation = displayRotation.load();

            if (width <= 0 || height <= 0)
                return;

            int touchX, touchY;

            switch (rotation)
            {
            case 0: // Portrait
                touchX = x * touchXMax / width + touchXMin;
                touchY = y * touchYMax / height + touchYMin;
                break;
            case 1: // Landscape (Clockwise)
                touchY = x * touchYMax / width + touchXMin;
                touchX = (height - y) * touchXMax / height + touchYMin;
                break;
            case 2: // Reverse Portrait
                touchX = (width - x) * touchXMax / width + touchXMin;
                touchY = (height - y) * touchYMax / height + touchYMin;
                break;
            case 3: // Landscape (Counter-Clockwise)
                touchY = (width - x) * touchYMax / width + touchXMin;
                touchX = y * touchXMax / height + touchYMin;
                break;
            default:
                return;
            }

            contacts[FAKE_CONTACT].posX = touchX;
            contacts[FAKE_CONTACT].posY = touchY;
            contacts[FAKE_CONTACT].enabled = true;

            touchChannel.put(true);
        }

        void sendTouchUp()
        {
            if (!touchStart.load() || !touchSend.load() || !contacts)
                return;

            touchSend = false;
            contacts[FAKE_CONTACT].posX = -1;
            contacts[FAKE_CONTACT].posY = -1;
            contacts[FAKE_CONTACT].enabled = false;

            touchChannel.put(true);
        }

        bool isTouchInZone(const TouchRect &zone)
        {
            if (!contacts || !touchStart.load())
                return false;

            std::lock_guard<std::mutex> lock(zoneCheckMutex);

            for (int i = 0; i < maxContacts; ++i)
            {
                if (contacts[i].enabled.load())
                {
                    int displayX = contacts[i].displayX.load();
                    int displayY = contacts[i].displayY.load();

                    if (displayX >= 0 && displayY >= 0 && zone.contains(displayX, displayY))
                    {
                        return true;
                    }
                }
            }
            return false;
        }

        std::vector<bool> checkMultipleZones(const std::vector<TouchRect> &zones)
        {
            std::vector<bool> results(zones.size(), false);

            if (!contacts || !touchStart.load())
                return results;

            std::lock_guard<std::mutex> lock(zoneCheckMutex);

            std::vector<std::pair<int, int>> activePoints;
            for (int i = 0; i < maxContacts; ++i)
            {
                if (contacts[i].enabled.load())
                {
                    int displayX = contacts[i].displayX.load();
                    int displayY = contacts[i].displayY.load();
                    if (displayX >= 0 && displayY >= 0)
                    {
                        activePoints.push_back({displayX, displayY});
                    }
                }
            }

            for (size_t zoneIdx = 0; zoneIdx < zones.size(); ++zoneIdx)
            {
                for (const auto &point : activePoints)
                {
                    if (zones[zoneIdx].contains(point.first, point.second))
                    {
                        results[zoneIdx] = true;
                        break;
                    }
                }
            }

            return results;
        }

        std::vector<std::pair<int, int>> getActiveTouchPoints()
        {
            std::vector<std::pair<int, int>> points;

            if (!contacts || !touchStart.load())
                return points;

            std::lock_guard<std::mutex> lock(zoneCheckMutex);

            for (int i = 0; i < maxContacts; ++i)
            {
                if (contacts[i].enabled.load())
                {
                    int displayX = contacts[i].displayX.load();
                    int displayY = contacts[i].displayY.load();
                    if (displayX >= 0 && displayY >= 0)
                    {
                        points.push_back({displayX, displayY});
                    }
                }
            }

            return points;
        }

    private:
        void convertTouchToDisplay(int touchX, int touchY, int &displayX, int &displayY)
        {
            int width = displayWidth.load();
            int height = displayHeight.load();
            int rotation = displayRotation.load();

            if (width <= 0 || height <= 0)
            {
                displayX = displayY = -1;
                return;
            }

            float normalizedX = float(touchX - touchXMin) / float(touchXMax - touchXMin);
            float normalizedY = float(touchY - touchYMin) / float(touchYMax - touchYMin);

            switch (rotation)
            {
            case 0: // Portrait
                displayX = normalizedX * width;
                displayY = normalizedY * height;
                break;
            case 1: // Landscape (90° clockwise)
                displayX = normalizedY * width;
                displayY = (1.0f - normalizedX) * height;
                break;
            case 2: // Reverse Portrait (180°)
                displayX = (1.0f - normalizedX) * width;
                displayY = (1.0f - normalizedY) * height;
                break;
            case 3: // Landscape (270° clockwise)
                displayX = (1.0f - normalizedY) * width;
                displayY = normalizedX * height;
                break;
            default:
                displayX = displayY = -1;
            }
        }

        void updateDisplayCoordinates()
        {
            if (!contacts)
                return;

            for (int i = 0; i < maxContacts; ++i)
            {
                if (contacts[i].enabled.load())
                {
                    int touchX = contacts[i].posX.load();
                    int touchY = contacts[i].posY.load();

                    if (touchX >= 0 && touchY >= 0)
                    {
                        int displayX, displayY;
                        convertTouchToDisplay(touchX, touchY, displayX, displayY);
                        contacts[i].displayX = displayX;
                        contacts[i].displayY = displayY;
                    }
                }
            }
        }

        void eventReaderThread()
        {
            int currSlot = 0;
            bool hasSyn = false;
            struct input_event evt;

            while (!shouldStop.load() && touchDeviceFd >= 0)
            {
                ssize_t bytesRead = read(touchDeviceFd, &evt, sizeof(evt));
                if (bytesRead != sizeof(evt))
                {
                    if (bytesRead < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
                        break;
                    continue;
                }

                std::lock_guard<std::mutex> lock(touchSynMtx);

                switch (evt.type)
                {
                case EV_SYN:
                    if (evt.code == SYN_REPORT)
                    {
                        hasSyn = true;
                        updateDisplayCoordinates();
                    }
                    break;
                case EV_ABS:
                    if (currSlot >= 0 && currSlot < maxContacts && contacts)
                    {
                        switch (evt.code)
                        {
                        case ABS_MT_SLOT:
                            currSlot = evt.value;
                            break;
                        case ABS_MT_TRACKING_ID:
                            if (currSlot < maxContacts)
                            {
                                contacts[currSlot].enabled = (evt.value != -1);
                                if (!contacts[currSlot].enabled.load())
                                {
                                    contacts[currSlot].posX = -1;
                                    contacts[currSlot].posY = -1;
                                    contacts[currSlot].displayX = -1;
                                    contacts[currSlot].displayY = -1;
                                }
                            }
                            break;
                        case ABS_MT_POSITION_X:
                            if (currSlot < maxContacts)
                                contacts[currSlot].posX = evt.value;
                            break;
                        case ABS_MT_POSITION_Y:
                            if (currSlot < maxContacts)
                                contacts[currSlot].posY = evt.value;
                            break;
                        }
                    }
                    break;
                }

                if (hasSyn)
                {
                    touchChannel.put(true);
                    hasSyn = false;
                }
            }
        }

        void eventWriterThread()
        {
            while (!shouldStop.load())
            {
                bool sync;
                if (!touchChannel.get(sync) || !sync)
                    continue;

                std::lock_guard<std::mutex> lock(touchSynMtx);

                if (!contacts || uInputTouchFd < 0)
                    break;

                int nextSlot = 0;
                for (int i = 0; i < maxContacts; i++)
                {
                    if (contacts[i].enabled.load() &&
                        contacts[i].posX.load() > 0 &&
                        contacts[i].posY.load() > 0)
                    {
                        writeEvent(uInputTouchFd, EV_ABS, ABS_MT_POSITION_X, contacts[i].posX.load());
                        writeEvent(uInputTouchFd, EV_ABS, ABS_MT_POSITION_Y, contacts[i].posY.load());
                        writeEvent(uInputTouchFd, EV_ABS, ABS_MT_TRACKING_ID, i);
                        writeEvent(uInputTouchFd, EV_SYN, SYN_MT_REPORT, 0x0);
                        nextSlot++;
                    }
                }

                bool btnDown = isBtnDown.load();
                if (nextSlot == 0 && btnDown)
                {
                    isBtnDown = false;
                    writeEvent(uInputTouchFd, EV_SYN, SYN_MT_REPORT, 0x0);
                    writeEvent(uInputTouchFd, EV_KEY, BTN_TOUCH, 0x0);
                }
                else if (nextSlot == 1 && !btnDown)
                {
                    isBtnDown = true;
                    writeEvent(uInputTouchFd, EV_KEY, BTN_TOUCH, 0x1);
                }

                writeEvent(uInputTouchFd, EV_SYN, SYN_REPORT, 0x0);
            }
        }

        void writeEvent(int fd, int type, int code, int value)
        {
            if (fd < 0)
                return;

            struct input_event inputEvent = {};
            inputEvent.type = type;
            inputEvent.code = code;
            inputEvent.value = value;

            write(fd, &inputEvent, sizeof(input_event));
        }

        bool HasSpecificAbs(int device_fd, unsigned int key)
        {
            size_t nchar = ABS_MAX / 8 + 1;
            unsigned char bits[nchar];

            if (ioctl(device_fd, EVIOCGBIT(EV_ABS, sizeof(bits)), &bits) < 0)
                return false;

            return bits[key / 8] & (1 << (key % 8));
        }

        bool isCharDevice(const char *path)
        {
            struct stat st;
            if (stat(path, &st) == -1)
                return false;

            return S_ISCHR(st.st_mode);
        }

        int getTouchDevice()
        {
            int fd = -1;
            struct dirent *entry;
            const char *input_path = "/dev/input";

            DIR *dir = opendir(input_path);
            if (!dir)
            {
                perror("Cannot open /dev/input directory");
                return -1;
            }

            while ((entry = readdir(dir)))
            {
                if (strncmp(entry->d_name, "event", 5) != 0)
                    continue;

                char devname[PATH_MAX];
                int ret = snprintf(devname, sizeof(devname), "%s/%s", input_path, entry->d_name);
                if (ret >= sizeof(devname))
                {
                    continue;
                }

                if (!isCharDevice(devname))
                    continue;

                fd = open(devname, O_RDONLY | O_NONBLOCK);
                if (fd < 0)
                    continue;

                if (!HasSpecificAbs(fd, ABS_MT_SLOT) ||
                    !HasSpecificAbs(fd, ABS_MT_POSITION_X) ||
                    !HasSpecificAbs(fd, ABS_MT_POSITION_Y) ||
                    !HasSpecificAbs(fd, ABS_MT_TRACKING_ID))
                {
                    close(fd);
                    fd = -1;
                    continue;
                }

                printf("Found touch device: %s\n", devname);
                break;
            }

            closedir(dir);

            if (fd < 0)
            {
                printf("No suitable touch device found in %s\n", input_path);
            }

            return fd;
        }
    };

    static TouchInputManager touchManager;

    void touchInputStart() { touchManager.start(); }
    void touchInputStop() { touchManager.stop(); }
    void sendTouchMove(int x, int y) { touchManager.sendTouchMove(x, y); }
    void sendTouchUp() { touchManager.sendTouchUp(); }
    void setDisplayInfo(int width, int height, int rotation)
    {
        touchManager.setDisplayInfo(width, height, rotation);
    }

    bool isTouchInZone(const TouchRect &zone)
    {
        return touchManager.isTouchInZone(zone);
    }

    std::vector<bool> checkMultipleZones(const std::vector<TouchRect> &zones)
    {
        return touchManager.checkMultipleZones(zones);
    }

    std::vector<std::pair<int, int>> getActiveTouchPoints()
    {
        return touchManager.getActiveTouchPoints();
    }

    TouchRect createZoneFromCenter(int centerX, int centerY, int radius)
    {
        return TouchRect::fromCenter(centerX, centerY, radius);
    }

    TouchRect createZoneFromCenter(int centerX, int centerY, int radiusX, int radiusY)
    {
        return TouchRect::fromCenter(centerX, centerY, radiusX, radiusY);
    }

    bool isTouchInCenterZone(int centerX, int centerY, int radius)
    {
        return touchManager.isTouchInZone(TouchRect::fromCenter(centerX, centerY, radius));
    }

    bool isTouchInCenterZone(int centerX, int centerY, int radiusX, int radiusY)
    {
        return touchManager.isTouchInZone(TouchRect::fromCenter(centerX, centerY, radiusX, radiusY));
    }

    struct CenterZone
    {
        int centerX, centerY, radius;
        CenterZone(int cx, int cy, int r) : centerX(cx), centerY(cy), radius(r) {}
    };

    std::vector<bool> checkMultipleCenterZones(const std::vector<CenterZone> &centerZones)
    {
        std::vector<TouchRect> zones;
        zones.reserve(centerZones.size());

        for (const auto &cz : centerZones)
        {
            zones.push_back(TouchRect::fromCenter(cz.centerX, cz.centerY, cz.radius));
        }

        return touchManager.checkMultipleZones(zones);
    }
}

#endif // TOUCH_H