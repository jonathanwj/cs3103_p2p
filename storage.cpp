// storage.cpp
#include "storage.h"
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h>

Chunk::Chunk(void *ptrToChunkData, size_t size, size_t count, std::string filename){
    chunkHeader = completeChunk;
    this->filename = filename;
    completeChunkSize = (size*count) / sizeof(char);
    memcmp(completeChunk, ptrToChunkData, completeChunkSize);
    chunkContent = completeChunk + chunkHeaderSize;
    chunkContentSize = parseInt32(chunkHeader + 4);
    chunkNumber = parseInt32(chunkHeader);
    finalFlag = chunkHeader[8];
}

Chunk::Chunk(void *ptrToChunkContent, size_t contentSize, size_t contentCount, int chunkNumber, bool finalFlag, std::string filename){
    chunkHeader = completeChunk;
    serializeInt32(chunkHeader, chunkNumber);
    this->chunkNumber = chunkNumber;
    this->filename = filename;
    chunkContentSize = (contentSize*contentCount) / sizeof(char);
    serializeInt32(chunkHeader + 4, chunkContentSize);
    chunkHeader[8] = finalFlag;
    completeChunkSize = chunkHeaderSize + chunkContentSize;
    chunkContent = completeChunk + chunkHeaderSize;
    memcmp(chunkContent, ptrToChunkContent, chunkContentSize);
}

int Chunk::getCompleteChunkSize(){return completeChunkSize;}
int Chunk::getChunkNumber(){return chunkNumber;}
int Chunk::getChunkContentSize(){return chunkContentSize;}
bool Chunk::isFinalChunk(){return finalFlag;} 
char * Chunk::getCompleteChunk(){return completeChunk;}

Storage::Storage(std::string pathToDownloadFolder)
{
    this->pathToDownloadFolder = pathToDownloadFolder;
    // create download directory
    if(!doesFileExist(pathToDownloadFolder)){
        // TODO CHANGE TO WINDOWS
        if (mkdir(pathToDownloadFolder.c_str(), 0777) == -1) {
            std::cerr << "Error :  " << strerror(errno) << std::endl; 
        } else {
            std::cout << "Directory created"; 
        }
    }
}

int Storage::saveChunk(void *ptrToChunk, size_t size, size_t count, std::string filename)
{
    std::string pathToFileCompleted = pathToDownloadFolder + "/" + filename;
    std::string pathToFileOfDownloading = pathToDownloadFolder + "/" + filename + ".p2pdownloading";
    // chunks being saved should be incomplete files

    // file is complete
    if(doesFileExist(pathToFileCompleted)) {
        // should not be saving a chunk to a completed file
        return -1;
    }

    //if file does not exist create new .p2pdownloading file
    if(!doesFileExist(pathToFileOfDownloading)){
        // TODO CHANGE TO WINDOWS
        std::ofstream outfile (pathToFileOfDownloading);
        outfile.close();
    }

    Chunk * chunkToSave = new Chunk(ptrToChunk,size,count,filename);

    bool finalChunkFound = false;
    int finalChunkNumber = -1;

    int chunkNumberToSave = chunkToSave->getChunkNumber();
    // if saving chunk is final chunk
    if(chunkToSave->isFinalChunk() == true){
        finalChunkFound = true;
        finalChunkNumber = chunkNumberToSave;
    }

    char readChunk[fixedChunkSizeWithHeader];

    std::ifstream is(pathToFileOfDownloading);
    int currChunkNumber;
    while (is.peek() != std::ifstream::traits_type::eof()){ // loop and search
        is.read(readChunk,fixedChunkSizeWithHeader);
        Chunk * chunk = new Chunk(readChunk,sizeof(char),fixedChunkSizeWithHeader,filename);

        currChunkNumber = chunk->getChunkNumber();

        if(currChunkNumber == chunkNumberToSave) {
            // if chunk to save is already downloaded
            return -1;
        }

        if(chunk->isFinalChunk() == true){
            // if final chunk
            finalChunkFound = true;
            finalChunkNumber = currChunkNumber;
        }
        delete chunk;
    }
    is.close();

    // append chunk to end of file
    std::ofstream myfile;
    myfile.open (pathToFileOfDownloading, std::ios::app);
    myfile.write( (char*)chunkToSave->getCompleteChunk(), chunkToSave->getCompleteChunkSize() );    
    myfile.close();

    if(!finalChunkFound) {
        return 1;
    }

    // check if all chunks are downloaded
    bool * chunkFlags = (bool*) malloc(finalChunkNumber + 1);
    int i;
    for(i=1; i<=finalChunkNumber; i++){
        chunkFlags[i] = false;
    }

    std::ifstream is2(pathToFileOfDownloading);
    while (is2.peek() != std::ifstream::traits_type::eof()){ 
        is2.read(readChunk,fixedChunkSizeWithHeader);
        Chunk * chunk = new Chunk(readChunk,sizeof(char),fixedChunkSizeWithHeader,filename);
        chunkFlags[chunk->getChunkNumber()] = true;
        delete chunk;
    }
    is2.close();

    bool fileCompleted = true;
    for(i=1; i<=finalChunkNumber; i++){
        if(chunkFlags[i] == false) {
            //std::cout<<i;
            fileCompleted = false;
            break;
        }
    }

    free (chunkFlags);

    if(fileCompleted) {
        //std::cout << "file completed\n";
        sortAndUpdateFullyDownloadedFile(filename);
    } else {
        //std::cout << "file not completed\n";
    }
 

    //std::string temp = pathToFile.substr(pathToFile.find_last_of(".") + 1);
    //std::cout<<pathToFileOfDownloading;
    //std::cout<<"\n";

    return 1;
}

void Storage::sortAndUpdateFullyDownloadedFile(std::string filename){
    std::string pathToFileCompleted = pathToDownloadFolder + "/" + filename;
    std::string pathToFileOfDownloading = pathToDownloadFolder + "/" + filename + ".p2pdownloading";

    char readChunkContent[fixedChunkContentSize];
    char readChunkHeader[fixedChunkHeaderSize];
    std::ifstream is(pathToFileOfDownloading);
    int count = 1;
    int fileSize = 0;
    while (is.peek() != std::ifstream::traits_type::eof()){ // loop and search
        is.read(readChunkHeader,fixedChunkHeaderSize);
        int chunkContentSize = parseInt32(readChunkHeader+4);
        is.read(readChunkContent,chunkContentSize);
        // int chunkNumber = parseInt32(readChunkHeader);
        fileSize += chunkContentSize;
        count++;
    }
    // std::cout<<count;
    // std::cout<<"\n";
    is.close();

    std::ofstream outfile;
    outfile.open (pathToFileCompleted);
    std::ifstream is2(pathToFileOfDownloading);
    while (is2.peek() != std::ifstream::traits_type::eof()){ // loop and search
        is2.read(readChunkHeader,fixedChunkHeaderSize);
        int chunkContentSize = parseInt32(readChunkHeader+4);
        is2.read(readChunkContent,chunkContentSize);
        int chunkNumber = parseInt32(readChunkHeader);
        outfile.seekp ((chunkNumber-1) * fixedChunkContentSize);
        outfile.write (readChunkContent,chunkContentSize);

    }
    //std::cout<<"\n";
    is2.close();
    outfile.close();

}

int Storage::getChunk(void *ptrToFillWithChunkData, std::string filename, int chunkNumber, size_t * chunkTotalByteSize)
{
    if(chunkNumber <= 0){
        // chunk number should be more than 0
        return -1;
    }
    std::string pathToFileCompleted = pathToDownloadFolder + "/" + filename;
    std::string pathToFileOfDownloading = pathToDownloadFolder + "/" + filename + ".p2pdownloading";
    // file does not exist
    if(!doesFileExist(pathToFileCompleted) && !doesFileExist(pathToFileOfDownloading)) {
        // should not be saving a chunk to a completed file
        return -1;
    }

    if(doesFileExist(pathToFileCompleted)){
        // get from completed file
        std::ifstream is(pathToFileCompleted);
        char readChunkContent[fixedChunkContentSize];
        char readChunkHeader[fixedChunkHeaderSize];
        int count = 1;
        bool isFinalChunk = false;
        while (is.peek() != std::ifstream::traits_type::eof()){ // loop getting chunk
            is.read(readChunkContent,fixedChunkContentSize);
            // set chunk number
            serializeInt32(readChunkHeader, count);
            //set chunk content size
            int chunkContentSize = is.gcount();
            serializeInt32(readChunkHeader+4, chunkContentSize);
            // set final flag
            isFinalChunk = false;
            if(count == chunkNumber){
                is.peek();
                if(is.eof()){
                    isFinalChunk = true;
                }

                Chunk* chunk = new Chunk(readChunkContent, sizeof(char), chunkContentSize, count, isFinalChunk, filename);

                memcpy(ptrToFillWithChunkData, chunk->getCompleteChunk(), chunk->getCompleteChunkSize());
                delete chunk;
                is.close();
                return 1;
            }
            count++;
        }
        is.close();
        return -1;

    } else if(doesFileExist(pathToFileOfDownloading)) {
        // get from incomplete file
        char readChunk[fixedChunkSizeWithHeader];
        char readChunkContent[fixedChunkContentSize];
        char readChunkHeader[fixedChunkHeaderSize];
        std::ifstream is(pathToFileOfDownloading);
        while (is.peek() != std::ifstream::traits_type::eof()){ // loop and search
            is.read(readChunkHeader,fixedChunkHeaderSize);
            int chunkContentSize = parseInt32(readChunkHeader+4);
            is.read(readChunkContent,chunkContentSize);
            int savedChunkNumber = parseInt32(readChunkHeader);
            if(chunkNumber == savedChunkNumber){
                Chunk* chunk = new Chunk(readChunk, sizeof(char), fixedChunkHeaderSize+chunkContentSize, filename);
                memcpy(ptrToFillWithChunkData, chunk->getCompleteChunk(), chunk->getCompleteChunkSize());
                delete chunk;
                is.close();
                return 1;
            }
        }
        is.close();

    }
    return -1;
}

void serializeInt32(char * buf, int32_t val)
{
    memcpy(buf, &val, 4);
}

int32_t parseInt32(char * buf)
{
    int32_t val;
    memcpy(&val, buf, 4);
    return val;
}

bool Storage::doesFileExist (const std::string& name) {
    std::ifstream f(name.c_str());
    return f.good();
}

int main()
{
    Storage *stor = new Storage("./test");
    char temp[2058];
    size_t totalChunkSize;
    size_t * chunkSizeRecieved = &totalChunkSize;
    while(1){
        int i = rand() % 2000 + 1; // random test for file size less than 2000 * chunk content size
        int work = stor->getChunk(temp,"test.png",i, chunkSizeRecieved);
        if(work != -1){
            stor->saveChunk(temp, sizeof(char), totalChunkSize, "test1.out");
        }
    }

    return 0;
}