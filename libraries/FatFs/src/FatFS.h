#include <FS.h>
#include <FSImpl.h>
#include "ff15/ff.h"
#include "ff15/diskio.h"

#ifndef _FATFS_H_
#define _FATFS_H_

using namespace fs;

namespace fatfs_impl {

class FatFSFileImpl;
class FatFSDirImpl;

class FatFSConfig : public FSConfig {
public:
    static constexpr uint32_t FSId = 0x46464154; // "FFAT"
    const char* _label = NULL;
    FatFSConfig(bool autoFormat = true) : FSConfig(FSId, autoFormat) { }
    void setLabel(const char* label) { _label = label; }
};

class FatFsImpl : public FSImpl {
protected:
    friend class FatFSFileImpl;
    friend class FatFSDirImpl;
    bool _tryMount();
    FATFS* getFS() {
        return &_fat;
    }
    bool  _mounted;
    FATFS  _fat;
    FatFSConfig _cfg;
public:
    FatFsImpl() : _mounted(false) { }
    ~FatFsImpl() {
        if (_mounted) {
            f_mount(NULL /* unmount */, "", 0);
        }
    }
    bool begin() override;
    bool setConfig(const FSConfig &cfg) override;
    FileImplPtr open(const char* path, OpenMode openMode, AccessMode accessMode) override;
    DirImplPtr openDir(const char *path) override;
    bool exists(const char* path) override;
    bool rename(const char* pathFrom, const char* pathTo) override;
    bool info(FSInfo& info) override;
    bool info64(FSInfo64& info) override;
    bool remove(const char* path) override;
    bool mkdir(const char* path) override;
    bool rmdir(const char* path) override;
    void end() override;
    bool format() override;
    time_t getCreationTime() override;
    bool setLabel(const char* label);
    bool getLabel(String& label, DWORD &serNum);
    bool pathValid(const char *path); 
};

class FatFSFileImpl : public FileImpl {
public:
    FatFSFileImpl(FatFsImpl* fs, const char *name, std::shared_ptr<FIL> fd, int flags, time_t creation) : _fs(fs), _fd(fd), _opened(true), _flags(flags), _creation(creation) {
        _name = std::shared_ptr<char>(new char[strlen(name) + 1], std::default_delete<char[]>());
        strcpy(_name.get(), name);
    }

    ~FatFSFileImpl() override {
        if (_opened) {
            close();
        }
    }
    size_t write(const uint8_t *buf, size_t size) override;
    int read(uint8_t* buf, size_t size) override;
    void flush() override;
    bool seek(uint32_t pos, SeekMode mode) override;
    size_t position() const override;
    size_t size() const override;
    bool truncate(uint32_t size) override;
    void close() override;
    time_t getLastWrite() override;
    time_t getCreationTime() override;
    const char* name() const override {
        if (!_opened) {
            return nullptr;
        } else {
            const char *p = _name.get();
            const char *slash = strrchr(p, '/');
            return (slash && slash[1]) ? slash + 1 : p;
        }
    }
    const char* fullName() const override {
        return _opened ? _name.get() : nullptr;
    }
    bool isDirectory() const override;
    bool isFile() const override;

    static int _getFlags(OpenMode openMode, AccessMode accessMode) {
        int mode = 0;
        if (openMode & OM_CREATE) {
            mode |= FA_OPEN_ALWAYS; // they could also mean FA_CREATE_NEW
        }
        if (openMode & OM_APPEND) {
            mode |= FA_OPEN_APPEND;
        }
        if (openMode & OM_TRUNCATE) {
            mode |= FA_CREATE_ALWAYS;
        }
        if (accessMode & AM_READ) {
            mode |= FA_READ;
        }
        if (accessMode & AM_WRITE) {
            mode |= FA_WRITE;
        }
        return mode;
    }
protected:
    FIL *_getFD() const {
        return _fd.get();
    }

    FatFsImpl             *_fs;
    std::shared_ptr<FIL>  _fd;
    std::shared_ptr<char>        _name;
    bool                         _opened;
    int                          _flags;
    time_t                       _creation;
};

class FatFSDirImpl : public DirImpl {
public:
    FatFSDirImpl(const String& pattern, FatFsImpl* fs, std::shared_ptr<DIR> dir, const char *dirPath = nullptr)
        : _pattern(pattern), _fs(fs), _dir(dir), _dirPath(nullptr), _valid(false), _opened(true) {
        memset(&_lastFile, 0, sizeof(_lastFile));
        if (dirPath) {
            _dirPath = std::shared_ptr<char>(new char[strlen(dirPath) + 1], std::default_delete<char[]>());
            strcpy(_dirPath.get(), dirPath);
        }
    }

    ~FatFSDirImpl() override {
        if (_opened) {
            f_closedir(_getDir());
        }
    }

    FileImplPtr openFile(OpenMode openMode, AccessMode accessMode) override {
        if (!_valid) {
            return FileImplPtr();
        }
        int nameLen = 3; // Slashes, terminator
        nameLen += _dirPath.get() ? strlen(_dirPath.get()) : 0;
        nameLen += strlen(_lastFile.fname);
        char tmpName[nameLen];
        snprintf(tmpName, nameLen, "%s%s%s", _dirPath.get() ? _dirPath.get() : "", _dirPath.get() && _dirPath.get()[0] ? "/" : "", _lastFile.fname);
        auto ret = _fs->open((const char *)tmpName, openMode, accessMode);
        return ret;
    }

    const char* fileName() override {
        if (!_valid) {
            return nullptr;
        }
        return (const char*) _lastFile.fname;
    }

    size_t fileSize() override {
        if (!_valid) {
            return 0;
        }
        return _lastFile.fsize;
    }

    time_t fileTime() override {
        return 0;
    }

    time_t fileCreationTime() override {
        // OTW, none present
        return 0;
    }

    bool isFile() const override {
        // If it's not a directory we deem it to be a file
        return _valid && ((_lastFile.fattrib & AM_DIR) == 0);
    }

    bool isDirectory() const override {
        return _valid && ((_lastFile.fattrib & AM_DIR) != 0);
    }

    bool rewind() override {
        _valid = false;
        FRESULT rc = f_rewinddir(_getDir());
        return (rc == FR_OK);
    }

    bool next() override {
        const int n = _pattern.length();
        bool match;
        do {
            FRESULT rc = f_readdir(_getDir(), &_lastFile);
            _valid = (rc == FR_OK);
            match = (!n || !strncmp((const char*) _lastFile.fname, _pattern.c_str(), n));
        } while (_valid && !match);
        return _valid;
    }

protected:
    DIR *_getDir() const {
        return _dir.get();
    }
    String                      _pattern;
    FatFsImpl                   *_fs;
    std::shared_ptr<DIR>        _dir;
    std::shared_ptr<char>       _dirPath;
    FILINFO                     _lastFile;
    bool                        _valid;
    bool                        _opened;
};


}; // namespace close

/* from diskio_rp2040_flash.h. Needed for USB operations */
extern void disk_read_direct (BYTE* buff, LBA_t sector, UINT num_bytes);

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_FATFS)
extern FS FatFS;
using fatfs_impl::FatFSConfig;
#endif

#endif // _FATFS_H_