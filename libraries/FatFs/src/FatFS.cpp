#include "FatFS.h"
#include "ff15/diskio.h"

extern uint8_t _FS_start;
extern uint8_t _FS_end;
const size_t _FS_len = (uint32_t)&_FS_end - (uint32_t)&_FS_start + 1;

namespace fatfs_impl {

bool FatFsImpl::begin() {
    if (_mounted) {
        return true;
    }
    if (_tryMount()) {
        return true;
    }
    if (!_cfg._autoFormat || !format()) {
        return false;
    }
    if (!_tryMount()) {
        return false;
    }
    if (_cfg._label != NULL) {
        String curLabel;
        DWORD serNum;
        // Current label not equal to wanted label?
        if(getLabel(curLabel, serNum) && strcmp(curLabel.c_str(), _cfg._label) != 0) {
            return setLabel(_cfg._label);
        }
    }
    return true;
}

bool FatFsImpl::setConfig(const FSConfig &cfg) {
    if ((cfg._type != FatFSConfig::FSId) || _mounted) {
        return false;
    }
    _cfg = *static_cast<const FatFSConfig *>(&cfg);
    return true;
}


bool FatFsImpl::_tryMount() {
    FRESULT res;
    res = f_mount(&this->_fat, "", 1 /* force mount*/);
    if (res == FR_OK ) {
        _mounted = true;
    }
    return _mounted;
}

bool FatFsImpl::pathValid(const char *path) {
    // Current config only "8.3" filenames are valid, i.e., max 8 chars filename, dot, max 3 chars extension
    // https://learn.microsoft.com/en-us/openspecs/windows_protocols/ms-fscc/18e63b13-ba43-4f5f-a5b7-11e871b71f14
    // Do only an extremely simple check here: Every path component shall be at maximum 12 characters long.
    while (*path) {
        const char *slash = strchr(path, '/');
        if (!slash) {
            if (strlen(path) > 12) {
                // filename is too long
                return false;
            }
            break;
        }
        if ((slash - path) > 12) {
            // This subdir name too long
            return false;
        }
        path = slash + 1;
    }
    return true;
}

FileImplPtr FatFsImpl::open(const char *path, OpenMode openMode,
                            AccessMode accessMode) {
    if (!_mounted) {
        DEBUGV("FatFsImpl::open() called on unmounted FS\n");
        return FileImplPtr();
    }
    if (!path || !path[0]) {
        DEBUGV("FatFsImpl::open() called with invalid filename\n");
        return FileImplPtr();
    }
    if (!FatFsImpl::pathValid(path)) {
        DEBUGV("FatFsImpl::open() called with too long filename\n");
        return FileImplPtr();
    }

    int flags = FatFSFileImpl::_getFlags(openMode, accessMode);
    auto fd = std::make_shared<FIL>();

    if ((openMode & OM_CREATE) && strchr(path, '/')) {
        // For file creation, silently make subdirs as needed.  If any fail,
        // it will be caught by the real file open later on
        char *pathStr = strdup(path);
        if (pathStr) {
            // Make dirs up to the final fnamepart
            char *ptr = strchr(pathStr, '/');
            while (ptr) {
                *ptr = 0;
                if(!this->mkdir(pathStr)) {
                    free(pathStr);
                    return FileImplPtr();
                }
                *ptr = '/';
                ptr = strchr(ptr + 1, '/');
            }
        }
        free(pathStr);
    }

    time_t creation = 0;
    if (_timeCallback && (openMode & OM_CREATE)) {
        // O_CREATE means we *may* make the file, but not if it already exists.
        // See if it exists, and only if not update the creation time
        FRESULT rc = f_open(fd.get(), path, FA_OPEN_EXISTING);
        if (rc == FR_OK) {
            f_close(fd.get()); // It exists, don't update create time
        } else {
            creation = _timeCallback();  // File didn't exist or otherwise, so we're going to create this time
        }
    }

    FRESULT rc = f_open(fd.get(), path, flags);
    if (rc == FR_NO_PATH) {
        // To support the SD.openNextFile, a null FD indicates to the LittleFSFile this is just
        // a directory whose name we are carrying around but which cannot be read or written
        return std::make_shared<FatFSFileImpl>(this, path, nullptr, flags, creation);
    } else if (rc == 0) {
        f_sync(fd.get());
        return std::make_shared<FatFSFileImpl>(this, path, fd, flags, creation);
    } else {
        DEBUGV("FatFSDirImpl::openFile: rc=%d fd=%p path=`%s` openMode=%d accessMode=%d err=%d\n",
               (int) rc, fd.get(), path, openMode, accessMode, rc);
        return FileImplPtr();
    }
}

DirImplPtr FatFsImpl::openDir(const char *path) {
    if (!_mounted || !path) {
        return DirImplPtr();
    }
    char *pathStr = strdup(path); // Allow edits on our scratch copy
    // Get rid of any trailing slashes
    while (strlen(pathStr) && (pathStr[strlen(pathStr) - 1] == '/')) {
        pathStr[strlen(pathStr) - 1] = 0;
    }
    // At this point we have a name of "blah/blah/blah" or "blah" or ""
    // If that references a directory, just open it and we're done.
    FILINFO info;
    auto dir = std::make_shared<DIR>();
    FRESULT rc;
    const char *filter = "";
    if (!pathStr[0]) {
        // openDir("") === openDir("/")
        rc = f_opendir(dir.get(), "/");
        filter = "";
    } else if (f_stat(pathStr, &info) == FR_OK) {
        if (info.fattrib & AM_DIR) {
            // Easy peasy, path specifies an existing dir!
            rc = f_opendir(dir.get(), pathStr);
            filter = "";
        } else {
            // This is a file, so open the containing dir
            char *ptr = strrchr(pathStr, '/');
            if (!ptr) {
                // No slashes, open the root dir
                rc = f_opendir(dir.get(), "/");
                filter = pathStr;
            } else {
                // We've got slashes, open the dir one up
                *ptr = 0; // Remove slash, truncate string
                rc = f_opendir(dir.get(), pathStr);
                filter = ptr + 1;
            }
        }
    } else {
        // Name doesn't exist, so use the parent dir of whatever was sent in
        // This is a file, so open the containing dir
        char *ptr = strrchr(pathStr, '/');
        if (!ptr) {
            // No slashes, open the root dir
            rc = f_opendir(dir.get(), "/");
            filter = pathStr;
        } else {
            // We've got slashes, open the dir one up
            *ptr = 0; // Remove slash, truncate string
            rc = f_opendir(dir.get(), pathStr);
            filter = ptr + 1;
        }
    }
    if (rc != FR_OK) {
        DEBUGV("FatFSImpl::openDir: path=`%s` err=%d\n", path, rc);
        free(pathStr);
        return DirImplPtr();
    }
    auto ret = std::make_shared<FatFSDirImpl>(filter, this, dir, pathStr);
    free(pathStr);
    return ret;
}

bool FatFsImpl::exists(const char *path) {
    FILINFO info;
    return f_stat(path, &info) == FR_OK;
}

bool FatFsImpl::rename(const char *pathFrom, const char *pathTo) {
    return f_rename(pathFrom, pathTo) == FR_OK;
}

bool FatFsImpl::info(FSInfo &info) {
    DWORD num_clusters;
    FATFS* fs;
    if (!_mounted) {
        return false;
    }
    if (f_getfree("", &num_clusters, &fs) != FR_OK) {
        return false;
    }
    // max open files meaningless for this library, can open as many as you want
    info.maxOpenFiles = SIZE_MAX;
    // fill in rest of static and dynamic info
    info.blockSize = FF_MAX_SS;
    info.pageSize = FF_MAX_SS;
    info.maxPathLength = 12; // "8.3" filename format
    info.totalBytes = _FS_len;
    info.usedBytes = num_clusters * FF_MAX_SS;
    return true;
}

bool FatFsImpl::info64(FSInfo64& info) {
    // In the configured FAT system, none of the entries are 64-bit.
    FSInfo _info;
    if (!this->info(_info)) {
        return false;
    }
    info.blockSize = _info.blockSize;
    info.maxOpenFiles = _info.maxOpenFiles;
    info.maxPathLength = _info.maxPathLength;
    info.pageSize=  _info.pageSize;
    info.totalBytes = _info.totalBytes;
    info.usedBytes = _info.usedBytes;
    return true;
}

bool FatFsImpl::remove(const char *path) {
    if (!_mounted || !path || !path[0]) {
        return false;
    }
    return f_unlink(path) == FR_OK;
}

bool FatFsImpl::mkdir(const char *path) {
    if (!_mounted || !path || !path[0]) {
        return false;
    }
    return f_mkdir(path) == FR_OK;
}

bool FatFsImpl::rmdir(const char *path) {
    return this->remove(path);
}

void FatFsImpl::end() {
    if (!_mounted) {
        return;
    }
    f_mount(NULL /* unmount */, "", 0);
    _mounted = false;
}

bool FatFsImpl::format() {
    FRESULT res;
    const MKFS_PARM formatopt = {
        FM_ANY, /* FS type */
        1, /* n_fat*/
        0, /* align */
        0, /* n_root*/
        0  /* au_size */
    };
    uint8_t work_buf[FF_MAX_SS]; // needs to be at least one sector
    res = f_mkfs("0:", &formatopt, work_buf, sizeof(work_buf));
    return res == FR_OK;
}

bool FatFsImpl::setLabel(const char* label) {
    if (!_mounted) {
        return false;
    }
    FRESULT res;
    res = f_setlabel(label);
    return res == FR_OK;
}

bool FatFsImpl::getLabel(String& label, DWORD &serNum) {
    if (!_mounted) {
        return false;
    }
    // per http://elm-chan.org/fsw/ff/doc/getlabel.html
    // We don't have exFAT and no long file support enabled
    TCHAR labelBuf[13] = {};
    FRESULT res;
    // Label not yet set?
    res = f_getlabel("", labelBuf, &serNum);
    if (res == FR_OK) {
        label = String(labelBuf); // makes a copy
        return true;
    }  else {
        label = String(); // empty
        return false;
    }
}

#define N_SEC_TRACK 63			/* Sectors per track for determination of drive CHS */

time_t FatFsImpl::getCreationTime() {
    // neat trick: When f_mkfs() is called, it sets the "volume serial number"
    // to "sz_vol + GET_FATTIME();"
    // since we know sz_vol, we can recalculate GET_FATTIME() at the time of creation.
    DWORD serNum;
    String label;
    DWORD sz_vol;
    if(!this->getLabel(label, serNum)) {
        return (time_t) 0; // "we don't know"
    }
    // reconstruct sz_vol as mkfatfs would have done on disc creation time
    if(disk_ioctl(0, GET_SECTOR_COUNT, &sz_vol) != RES_OK) {
        return (time_t) 0;
    }
    if (sz_vol > N_SEC_TRACK) {
        sz_vol -= N_SEC_TRACK;	/* Estimated partition offset and size */
    }
    return (time_t)(serNum - sz_vol);
}

size_t FatFSFileImpl::write(const uint8_t *buf, size_t size)  {
    UINT bytesWritten = 0;
    f_write(_getFD(), buf, size, &bytesWritten); 
    return bytesWritten;
}
int FatFSFileImpl::read(uint8_t* buf, size_t size)  {
    UINT bytesRead = 0;
    f_read(_getFD(), buf, size, &bytesRead);
    return (int) bytesRead;
}
void FatFSFileImpl::flush()  {
    f_sync(_getFD());
}
bool FatFSFileImpl::seek(uint32_t pos, SeekMode mode)  {
    if(mode == SeekMode::SeekSet) {
        f_lseek(_getFD(), 0);
    }
    return false;
}
size_t FatFSFileImpl::position() const  {
    return f_tell(_getFD());
}
size_t FatFSFileImpl::size() const  { 
    return f_size(_getFD());
}
bool FatFSFileImpl::truncate(uint32_t size)  {
    if(f_lseek(_getFD(), size) == FR_OK) {
        return f_truncate(_getFD()) == FR_OK;
    }
    return false;
}
void FatFSFileImpl::close()  {
    f_close(_getFD());
    this->_opened = false;
}

// See http://elm-chan.org/fsw/ff/doc/sfileinfo.html
union fat_date_t {
    struct {
        uint8_t day:5;  // 1..31
        uint8_t month:4; // 1..12
        uint8_t year:7; // 0..127 (year since 1980)
    } fields;
    WORD as_word;
};

union fat_time_t {
    struct {
        uint8_t double_seconds:5; // 0..29 to map onto 0 to 58 seconds
        uint8_t minute:6; // 0..59 minutes
        uint8_t hour:5; // 0..23 hours
    } fields;
    WORD as_word;
};

time_t convert_fat_to_time(WORD fdate, WORD ftime) {
    struct tm tm {};
    fat_date_t fatDate {.as_word = fdate};    
    fat_time_t fatTime {.as_word = ftime};    
    // FAT: year since 1980, but "struct tm" needs since 1900
    tm.tm_year = fatDate.fields.year + 80;
    // FAT: month 1 = Jan, but "struct tm" month 0 = Jan
    tm.tm_mon = fatDate.fields.month - 1;
    // identitcal.
    tm.tm_mday = fatDate.fields.day;
    tm.tm_hour = fatTime.fields.hour;
    tm.tm_min = fatTime.fields.minute;
    tm.tm_sec = fatTime.fields.double_seconds * 2;
    return mktime(&tm);
}

time_t FatFSFileImpl::getLastWrite() {
    FILINFO info;
    if(f_stat(this->_name.get(), &info) != FR_OK) {
        return 0; // "unknown"
    }
    return convert_fat_to_time(info.fdate, info.ftime);
}

time_t FatFSFileImpl::getCreationTime()  { 
    // meaningless for this implementation, not tracked in FAT.
    return 0;
}
bool FatFSFileImpl::isDirectory() const {
    return false;
}

bool FatFSFileImpl::isFile() const {
    return true;
}

};

FS FatFS = FS(FSImplPtr(new fatfs_impl::FatFsImpl()));
