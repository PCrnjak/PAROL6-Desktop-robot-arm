# SerialFlash

SerialFlash provides low-latency, high performance access to SPI Flash memory with a filesystem-like interface for use with Arduino compatible boards.  Familiar file-based functions, similar to the SD library, are used to access data.

In-progress file write and erase operations do NOT block read access on other files.  SerialFlash automatically allocates files with Flash page and sector awareness, and supports suspending in-progress write and erase operations, to minimize read latency even while the Flash memory is "busy" writing data.

Performance oriented design does impose some usage limitations.  Files are created with a fixed size which can never change or grow.  Once created, files can not be renamed or deleted (except for erasing the entire chip).  Files begin with all bytes erased (255).  Each byte may be written only once.  Files created as erasable may be fully erased, to allow new data to be written.  Best performance is achieved by writing in 256 byte chunks, though individual bytes may be written.


## Hardware Compatibility

![W25Q128FV Chip](doc/w25q128fv.jpg)

These chips have been tested with SerialFlash:

    Winbond W25Q80BV    (http://www.adafruit.com/product/1564)
    Winbond W25Q64FV
    Winbond W25Q128FV
    Winbond W25Q256FV
    Micron N25Q512A
    Micron N25Q00AA
    Spansion S25FL127S
    Spansion S25FL256S
    Spansion S25FL512S

SerialFlash automatically detects SPI Flash chip type and capacity to automatically handle differences between supported chips.

## Accessing Files

### Open A File

    SerialFlashFile file;
    file = SerialFlash.open("filename.bin");
    if (file) {  // true if the file exists

### Read Data

    char buffer[256];
    file.read(buffer, 256);
    
### File Size & Positon

    file.size();
    file.position()
    file.seek(number);
    
### Write Data

    file.write(buffer, 256);
    
Several limitations apply to writing.  Only previously unwritten portions of the file may be written.  File sizes can never change.  Writes may only be done within the file's original size.

### Erase Data

    file.erase();
    
Only files created for erasing can be erased.  The entire file is erased to all 255 (0xFF) bytes, which allows the file to be written with new data.
    
## Managing Files

### Create New Files

    SerialFlash.create(filename, size);
    SerialFlash.createErasable(filename, size);
    
New files must be created using these funtions.  Each returns true if the file is successfully created, or false if not enough space is available.

Once created, files can never be renamed or deleted.  The file's size can never change.  Writing additional data can NOT grow the size of file.

Files created for erasing automatically increase in size to the nearest number of erasable blocks, resulting in a file that may be 4K to 128K larger than requested.

### Delete A File

    SerialFlash.remove(filename);

The actual space used by the file is not reclaimed.  However, a new file with this name may be created after the original is deleted.

### Check If A File Exists (without opening)

    SerialFlash.exists(filename);

### Directory Listing

    SerialFlash.opendir();
    SerialFlash.readdir(buffer, buflen, filelen);
    
A list of files stored in the Flash can be accessed with readdir(), which returns true for each file, or false to indicate no more files.

## Full Erase

    SerialFlash.erase();
    
    while (SerialFlash.ready() == false) {
       // wait, 30 seconds to 2 minutes for most chips
    }
