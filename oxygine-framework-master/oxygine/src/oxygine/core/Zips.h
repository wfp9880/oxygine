#pragma once
#include "../oxygine-include.h"
#include "FileSystem.h"
#include "../core/Mutex.h"
#include "../core/file.h"
#include "../minizip/unzip.h"
#include <unordered_map>
#include <vector>


namespace oxygine
{
    namespace file
    {
        struct file_entry
        {
            unzFile zp;
            char name[128];
            unz_file_pos pos;
            int refs;
        };

        class Zips
        {
        public:
            Zips();
            ~Zips();

            void reset();

            void add(const char* name);
            void add(const unsigned char* data, unsigned int size);
            void add(std::vector<char>& data);
            void remove(const char* name);

            bool read(const char* name, file::buffer& bf);
            bool read(const file_entry* entry, file::buffer& bf);
            bool isExists(const char* name);

            file_entry*         getEntryByName(const char* name);
            const char*         getZipFileName(int i) const { return _zps[i].name; }
            Mutex&              getMutex() { return _lock; }

        private:
            friend class ZipFileSystem;
            void read(unzFile zp);

            typedef std::unordered_map<std::string, file_entry> files;
            files _files;

            struct zpitem
            {
                unzFile handle;
                std::vector<char> data;
                char name[255];

                zpitem() : handle(0) { name[0] = 0; }
            };
            typedef std::vector<zpitem> zips;
            zips _zps;

            Mutex _lock;
        };

        bool read(file_entry* entry, file::buffer& bf);
    }
}