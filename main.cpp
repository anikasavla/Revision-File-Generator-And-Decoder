#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include <list>
#include <cassert>
using namespace std;

//Hash Table manual class
class HashTable {
private:
    //maximum buckets
    static const int hashGroups = 50000;
    vector<list<pair<string, int>>> hashTable;
public:
    HashTable();
    void insert(string sequence, int offset);
    int search(string sequence);
};

HashTable::HashTable(){
    hashTable.resize(hashGroups);
}
//using the key place the sequence and offset in the table
void HashTable::insert(string sequence, int offset) {
    int hashValue = hash<string>()(sequence) % hashGroups;
    list<pair<string, int>>::iterator it = hashTable[hashValue].begin();
    for (; it != hashTable[hashValue].end(); it++) {
        if (it->first == sequence) {
            it->second = offset;
            return;
        }
    }
    hashTable[hashValue].emplace_back(sequence, offset);
}

//search through the hash table for an item and return its offset if key finds mathc
int HashTable::search(string sequence){
    int hashValue = hash<string>()(sequence) % hashGroups;
    list<pair<string, int>>::iterator it = hashTable[hashValue].begin();
    for (; it != hashTable[hashValue].end(); it++) {
        if (it->first == sequence) {
            return it->second;
        }
    }
    return -1;
}

string readStream(istream& stream) {
    //collects every character in the stream and adds it to the string
    string fullStream;
    char character;

    while (stream.get(character)) {
        fullStream += character;
    }

    return fullStream;
}

//depending on what characters are in the sequence, choose the delimitir char
string findDelim(string sequence){
    string delim;
    string delimOptions[] = {"/", ":", "~", "\n", "{", "}", "]","[",".", "?", "=", "*", "`", "%", "&","$"};
    int delimOptionsSize = sizeof(delimOptions) / sizeof(delimOptions[0]);
    int delimOptionsI = 0;
    bool delimFound = true;
    while (delimFound && delimOptionsI < delimOptionsSize){
        delimFound = false;
        delim = delimOptions[delimOptionsI];
        for (int i = 0; i < sequence.size(); i++) {
            if (delim == sequence.substr(i,1)) {
                delimFound = true;
                break;
            }
        }
        delimOptionsI++;
    }
    return delim;
}

void createRevision(istream& fold, istream& fnew, ostream& frevision) {
    const int N = 8;
    
    string oldFile = readStream(fold);
    string newFile = readStream(fnew);
    string revision;
    
    HashTable hashTable;
    
    //For all consecutive N-character sequences in the old file's string, insert that N-character sequence and the offset F where it was found in the old file's string, into a table
    for (int offset = 0; offset <= oldFile.size() - N; offset ++) {
        hashTable.insert(oldFile.substr(offset, N), offset);
    }
    
    //start processing the new file's string, starting from j=0, until offset reaches the end of the string.
    int j = 0;
    string lastAdd = "";
    
    //while still within the new file
    while(j < newFile.size()){
        string sequence = newFile.substr(j, N);
        int offset = hashTable.search(sequence);
        //variation of sequence not found so add the first item and then try with the next sequence
        if (offset < 0) {
            lastAdd += sequence[0];
            j++;
        }
        // if the variation of the sequence found
        else {
            //the additions have not yet been added to the revision file
            if (!lastAdd.empty()) {
                //find a delim to use that isn't present in the sequence to add
                string delim = findDelim(lastAdd);
                revision += "+" + delim + lastAdd + delim;
                lastAdd = "";
            }
            
            int copyLength = offset;
            //while the characters are equal at the certain location
            while (copyLength < oldFile.size() && j < newFile.size() && oldFile[copyLength] == newFile[j]){
                    copyLength++;
                    j++;
            }
            revision += "#" + to_string(offset) + "," + to_string(copyLength - offset);
        }
    }
    //didn't finish adding items
    if (!lastAdd.empty()) {
        string delim = findDelim(lastAdd);
        revision += "+" + delim + lastAdd + delim;
    }
    frevision << revision;
}

bool getInt(istream& inf, int& n)
{
    char ch;
    if (!inf.get(ch)  ||  !isascii(ch)  ||  !isdigit(ch))
        return false;
    inf.unget();
    inf >> n;
    return true;
}

bool getCommand(istream& inf, char& cmd, char& delim, int& length, int& offset)
{
    if (!inf.get(cmd))
    {
        cmd = 'x';  // signals end of file
        return true;
    }
    switch (cmd)
    {
        case '+':
        return inf.get(delim).good();
        case '#':
        {
            char ch;
            return getInt(inf, offset) && inf.get(ch) && ch == ',' && getInt(inf, length);
        }
        case '\r':
        case '\n':
        return true;
        }
    return false;
}

bool revise(istream& fold, istream& frevision, ostream& fnew) {
    string oldFile = readStream(fold);
    
    string newFile;
    char cmd, delim;
    int length, offset;
    while (getCommand(frevision, cmd, delim, length, offset)) {
        switch (cmd) {
            //encountered the add command
            case '+':
            {
                //create a string to input the addition from oldFile for the newFile
                string addition;
                //from the delim to the next delim, puts in the text to addition
                getline(frevision, addition, delim);
                //adds the addition to the exisiting newFile
                newFile+= addition;
                break;
            }
            //encountered the copy command
            case '#':
            {
                //checks that the offset is within the file as is the length of the copy text so that there is something there to take from the oldFile and add to newFile
                if (offset <= oldFile.size() && offset + length <= oldFile.size()) {
                    // add the oldFile's content from the offset to the offset plus length
                    newFile+=oldFile.substr(offset, length);
                }
                else {
                    return false;
                }
                break;
            }
            //encountered the do nothing commands
            case '\r':
            case '\n':
                newFile += cmd;
                return true;
            // case that the end of the stream is reached
            case 'x':
                fnew << newFile;
                return true;
            default:
                return false;
        }
    }
    return false;
}


    void runtest(string oldtext, string newtext)
    {
        istringstream oldFile(oldtext);
        istringstream newFile(newtext);
        ostringstream revisionFile;
        createRevision(oldFile, newFile, revisionFile);
        string result = revisionFile.str();
        cout << "The revision file length is " << result.size()
             << " and its text is " << endl;
        cout << result << endl;

        oldFile.clear();   // clear the end of file condition
        oldFile.seekg(0);  // reset back to beginning of the stream
        istringstream revisionFile2(result);
        ostringstream newFile2;
        assert(revise(oldFile, revisionFile2, newFile2));
        assert(newtext == newFile2.str());
    }

    int main()
    {
        runtest("There's a bathroom on the right.",
                "There's a bad moon on the rise.");
        runtest("ABCDEFGHIJBLAHPQRSTUVPQRSTUV",
                "XYABCDEFGHIJBLETCHPQRSTUVPQRSTQQ/OK");
        cout << "All tests passed" << endl;
    }
