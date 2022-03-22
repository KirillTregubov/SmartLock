// Target board has QSPI Flash
#define MOUNT_POINT "flash"

#include "mbed.h"

#include "features/storage/filesystem/FileSystem.h"
#include "features/storage/filesystem/fat/FATFileSystem.h"
#include "features/storage/filesystem/littlefs/LittleFileSystem.h"
#include "unity/unity.h"

#define MAX_BLOCKDEVICE_SIZE (32*1024*1024)

FileSystem* set_filesystem(BlockDevice* bd)
{
    static LittleFileSystem flash("flash", bd);
    flash.set_as_default();

    return &flash;
}

void mbed_format_file(void)
{
    BlockDevice* bd = BlockDevice::get_default_instance();
    TEST_ASSERT_NOT_NULL_MESSAGE(bd, "no BlockDevice defined");

    mbed::bd_size_t size = bd->size();
    TEST_ASSERT_NOT_EQUAL_MESSAGE(0, size, "incorrect BlockDevice size");

    printf("BlockDevice size: %llu\r\n", size);

    if (size > MAX_BLOCKDEVICE_SIZE) {
        bd = new SlicingBlockDevice(bd, 0, MAX_BLOCKDEVICE_SIZE);
        TEST_ASSERT_NOT_NULL_MESSAGE(bd, "unable to slice default BlockDevice");

        size = bd->size();
        TEST_ASSERT_EQUAL_INT_MESSAGE(MAX_BLOCKDEVICE_SIZE, size, "incorrect SlicingBlockDevice size");

        printf("Adjusted BlockDevice size: %llu\r\n", size);
    }

    FileSystem* fs = set_filesystem(bd);
    TEST_ASSERT_NOT_NULL_MESSAGE(fs, "unable to create FileSystem");

    int result = fs->reformat(bd);
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, result, "could not format block device");
}

void mbed_write_file(const char* file, size_t offset, const unsigned char* data, size_t data_length, size_t block_size)
{
    char filename[255] = { 0 };
    snprintf(filename, 255, "/" MOUNT_POINT "/%s", file);

    FILE* output = fopen(filename, "w+");
    TEST_ASSERT_NOT_NULL_MESSAGE(output, "could not open file");

    int result = fseek(output, offset, SEEK_SET);
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, result, "could not seek to location");

    size_t index = 0;
    while (index < data_length)
    {
        size_t write_length = data_length - index;

        if (write_length > block_size)
        {
            write_length = block_size;
        }

        size_t written = fwrite(&data[index], sizeof(unsigned char), write_length, output);
        TEST_ASSERT_EQUAL_UINT_MESSAGE(write_length, written, "failed to write");

        index += write_length;
    }
    TEST_ASSERT_EQUAL_UINT_MESSAGE(index, data_length, "wrong length");

    result = fclose(output);
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, result, "could not close file");
}

void mbed_compare_file(const char* file, size_t offset, const unsigned char* data, size_t data_length, size_t block_size)
{
    char filename[255] = { 0 };
    snprintf(filename, 255, "/" MOUNT_POINT "/%s", file);

    FILE* output = fopen(filename, "r");
    TEST_ASSERT_NOT_NULL_MESSAGE(output, "could not open file");

    int result = fseek(output, offset, SEEK_SET);
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, result, "could not seek to location");

    char* buffer = (char*) malloc(block_size);
    TEST_ASSERT_NOT_NULL_MESSAGE(buffer, "could not allocate buffer");

    size_t index = 0;
    while (index < data_length)
    {
        uint32_t read_length = data_length - index;

        if (read_length > block_size)
        {
            read_length = block_size;
        }

        size_t read = fread(buffer, sizeof(char), read_length, output);
        TEST_ASSERT_EQUAL_MESSAGE(read, read_length, "failed to read");
        TEST_ASSERT_EQUAL_STRING_LEN_MESSAGE(buffer, &data[index], read_length, "character mismatch");

        index += read_length;
    }
    TEST_ASSERT_EQUAL_UINT_MESSAGE(index, data_length, "wrong length");

    result = fclose(output);
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, result, "could not close file");

    free(buffer);
}

size_t mbed_read_file(const char* file, size_t offset, unsigned char* buffer, size_t buffer_length)
{
    char filename[255] = { 0 };
    snprintf(filename, 255, "/" MOUNT_POINT "/%s", file);

    FILE* output = fopen(filename, "r");
    TEST_ASSERT_NOT_NULL_MESSAGE(output, "could not open file");

    int result = fseek(output, offset, SEEK_SET);
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, result, "could not seek to location");

    size_t read = fread(buffer, sizeof(char), buffer_length, output);

    result = fclose(output);
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, result, "could not close file");

    return read;
}