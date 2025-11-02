// main.cpp

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <openssl/sha.h> // OpenSSL is installed for SHA-1
#include <zlib.h>        // zlib is installed for compression
#include <cstring>
#include <cstdio>        // For perror
#include <cstdlib>       // For exit

using namespace std;
namespace fs = std::filesystem;

// Constants for .mygit structure
const string MYGIT_DIR = ".mygit";
const string OBJECTS_DIR = MYGIT_DIR + "/objects";
const string REFS_DIR = MYGIT_DIR + "/refs";
const string HEADS_DIR = REFS_DIR + "/heads";
const string HEAD_FILE = MYGIT_DIR + "/HEAD";
const string INDEX_FILE = MYGIT_DIR + "/index";


void printOutput(const string& message) {
    cout << message;
}
void printError(const string& message) {
    cerr << message;
}
void errorExit(const string& message) {
    printError("Error: " + message + "\n");
    perror("");
    exit(EXIT_FAILURE);
}

string sha1Hash(const string &data) {
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char *>(data.c_str()), data.size(), hash);
    stringstream ss;
    for (int i = 0; i < SHA_DIGEST_LENGTH; ++i)
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    return ss.str();
}
string compressData(const string &data) {
    uLongf compressedSize = compressBound(data.size());
    vector<char> compressed(compressedSize);
    if (compress(reinterpret_cast<Bytef *>(compressed.data()), &compressedSize,
                 reinterpret_cast<const Bytef *>(data.c_str()), data.size()) != Z_OK) {
        errorExit("Compression failed");
    }
    return string(compressed.data(), compressedSize);
}
string decompressData(const string &data) {
    // Estimate decompressed size
    uLongf decompressedSize = data.size() * 10;
    vector<char> decompressed(decompressedSize);
    if (uncompress(reinterpret_cast<Bytef *>(decompressed.data()), &decompressedSize,
                   reinterpret_cast<const Bytef *>(data.c_str()), data.size()) != Z_OK) {
        errorExit("Decompression failed");
    }
    return string(decompressed.data(), decompressedSize);
}
void init() {
    if (fs::exists(MYGIT_DIR)) {
        printError("Repository already initialized, can't re-initialise.\n");
        return;
    }

    fs::create_directory(MYGIT_DIR);
    fs::create_directory(OBJECTS_DIR);
    fs::create_directory(REFS_DIR);
    fs::create_directory(HEADS_DIR);
    ofstream head(HEAD_FILE);
    if (!head) {
        errorExit("Error creating HEAD file. Please recheck.");
    }
    head << "ref: refs/heads/master\n";
    head.close();

    ofstream index(INDEX_FILE);
    if (!index) {
        errorExit("Error creating index file.");
    }
    index.close();

    printOutput("Initialized empty mygit repository in " + fs::absolute(MYGIT_DIR).string() + "\n");
}

string writeObject(const string &data) {
    string hash = sha1Hash(data);
    string path = OBJECTS_DIR + "/" + hash;

    if (!fs::exists(path)) {
        // Compress the data before storing
        string compressed = compressData(data);
        ofstream obj(path, ios::binary);
        if (!obj) {
            errorExit("Error writing object file.");
        }
        obj.write(compressed.c_str(), compressed.size());
        obj.close();
    }

    return hash;
}

string readObject(const string &hash) {
    string path = OBJECTS_DIR + "/" + hash;
    if (!fs::exists(path)) {
        errorExit("Object does not exist.");
    }

    ifstream obj(path, ios::binary);
    if (!obj) {
        errorExit("Error reading object file.");
    }

    stringstream buffer;
    buffer << obj.rdbuf();
    string compressed = buffer.str();
    obj.close();

    return decompressData(compressed);
}

// Function to handle the 'hash-object' command
void hashObject(bool writeFlag, const string &filePath) {
    if (!fs::exists(filePath)) {
        printError("File does not exist.\n");
        return;
    }

    ifstream file(filePath, ios::binary);
    if (!file) {
        errorExit("Error opening file.");
    }

    stringstream buffer;
    buffer << file.rdbuf();
    string content = buffer.str();
    file.close();

    string hash = sha1Hash(content);
    printOutput(hash + "\n");

    if (writeFlag) {
        writeObject(content);
        printOutput("Object written to repository.\n");
    }
}
void catFile(const string &flag, const string &hash) {
    string data = readObject(hash);

    if (flag == "-p") {
        printOutput(data);
    } else if (flag == "-s") {
        printOutput(to_string(data.size()) + "\n");
    } else if (flag == "-t") {
        if (data.find("tree ") != string::npos) {
            printOutput("tree\n");
        } else {
            printOutput("blob\n");
        }
    } else {
        printError("Unknown flag.\n");
    }
}
string writeTree(const fs::path &path = ".") {
    stringstream treeContent;
    for (const auto &entry : fs::directory_iterator(path)) {
        string filename = entry.path().filename().string();
        if (filename == ".mygit" || filename == "mygit")
            continue; 

        string mode;
        string type;
        string hash;

        if (fs::is_directory(entry.path())) {
            mode = "40000";
            type = "tree";
            hash = writeTree(entry.path());
        } else if (fs::is_regular_file(entry.path())) {
            mode = "100644";
            type = "blob";

            ifstream file(entry.path(), ios::binary);
            if (!file) {
                printError("Error opening file " + entry.path().string() + "\n");
                continue;
            }
            stringstream buffer;
            buffer << file.rdbuf();
            string content = buffer.str();
            file.close();

            hash = writeObject(content);
        } else {
            continue; // Skip other types
        }

        treeContent << mode << " " << type << " " << hash << " " << filename << "\n";
    }

    string treeStr = treeContent.str();
    return writeObject(treeStr);
}

void writeTreeCommand() {
    string treeHash = writeTree();
    printOutput(treeHash + "\n");
}


void lsTree(const string &flag, const string &treeHash) {
    string treeData = readObject(treeHash);
    stringstream ss(treeData);
    string line;

    while (getline(ss, line)) {
        stringstream ls;
        ls << line;
        string mode, type, hash, name;
        ls >> mode >> type >> hash;
        getline(ls, name);
        name.erase(0, name.find_first_not_of(" ")); // Trim leading spaces

        if (flag == "--name-only") {
            printOutput(name + "\n");
        } else {
            printOutput(mode + " " + type + " " + hash + " " + name + "\n");
        }
    }
}


map<string, string> readIndex() {
    map<string, string> indexMap;
    ifstream index(INDEX_FILE);
    if (!index) {
        return indexMap;
    }

    string line;
    while (getline(index, line)) {
        stringstream ss(line);
        string path, hash;
        ss >> path >> hash;
        indexMap[path] = hash;
    }

    index.close();
    return indexMap;
}


void writeIndex(const map<string, string> &indexMap) {
    ofstream index(INDEX_FILE, ios::trunc);
    if (!index) {
        errorExit("Error writing to index file.");
    }

    for (const auto &entry : indexMap) {
        index << entry.first << " " << entry.second << "\n";
    }

    index.close();
}

void add(const vector<string> &files) {
    map<string, string> indexMap = readIndex();

    for (const auto &file : files) {
        if (file == "mygit" || file == ".mygit") {
            printError("Skipping " + file + "\n");
            continue;
        }

        if (!fs::exists(file)) {
            printError("File " + file + " does not exist.\n");
            continue;
        }

        if (fs::is_directory(file)) {
           
            for (const auto &entry : fs::recursive_directory_iterator(file)) {
                if (fs::is_regular_file(entry.path())) {
                    string filePath = entry.path().string();
                    if (filePath.find(".mygit") != string::npos || filePath.find("mygit") != string::npos) {
                        continue; // Skip .mygit directory and mygit executable
                    }

                    ifstream f(filePath, ios::binary);
                    if (!f) {
                        printError("Error opening file " + filePath + ".\n");
                        continue;
                    }

                    stringstream buffer;
                    buffer << f.rdbuf();
                    string content = buffer.str();
                    f.close();

                    string hash = sha1Hash(content);
                    writeObject(content);
                    indexMap[filePath] = hash;
                    printOutput("Added " + filePath + "\n");
                }
            }
        } else {
            ifstream f(file, ios::binary);
            if (!f) {
                printError("Error opening file " + file + ".\n");
                continue;
            }

            stringstream buffer;
            buffer << f.rdbuf();
            string content = buffer.str();
            f.close();

            string hash = sha1Hash(content);
            writeObject(content);
            indexMap[file] = hash;
            printOutput("Added " + file + "\n");
        }
    }

    writeIndex(indexMap);
}


string getCurrentBranch() {
    ifstream head(HEAD_FILE);
    if (!head) {
        errorExit("HEAD file not found.");
    }

    string refLine;
    getline(head, refLine);
    head.close();

    if (refLine.find("ref: ") == 0) {
        return refLine.substr(5);
    } else {
        return "";
    }
}

string getLatestCommit(const string &branch) {
    fs::path branchPath = fs::path(MYGIT_DIR) / branch;
    if (!fs::exists(branchPath)) {
        return "";
    }

    ifstream ref(branchPath);
    if (!ref) {
        errorExit("Error reading branch reference.");
    }

    string commitSha;
    getline(ref, commitSha);
    ref.close();

    return commitSha;
}


void commit(const string &message) {
    map<string, string> indexMap = readIndex();

    string treeHash = writeTree();

    string branch = getCurrentBranch();
    if (branch.empty()) {
        errorExit("No branch is set in HEAD.");
    }
    fs::path branchPath = fs::path(MYGIT_DIR) / branch;
    fs::create_directories(branchPath.parent_path());
    if (!fs::exists(branchPath)) {
        ofstream outfile(branchPath); 
        outfile.close();
    }

    string parent = getLatestCommit(branch);
    auto now = chrono::system_clock::now();
    time_t nowTime = chrono::system_clock::to_time_t(now);
    string timestamp = to_string(nowTime);

    stringstream commitContent;
    commitContent << "tree " << treeHash << "\n";
    if (!parent.empty()) {
        commitContent << "parent " << parent << "\n";
    }
    commitContent << "author YourName <you@example.com> " << timestamp << " +0000\n";
    commitContent << "committer YourName <you@example.com> " << timestamp << " +0000\n\n";
    commitContent << message << "\n";

    string commitStr = commitContent.str();
    string commitHash = writeObject(commitStr); 
    ofstream ref(branchPath, ios::trunc);
    if (!ref) {
        errorExit("Error updating branch reference.");
    }
    ref << commitHash << "\n";
    ref.close();

    printOutput("Committed as " + commitHash + "\n");
}
void log() {
    string branch = getCurrentBranch();
    if (branch.empty()) {
        printError("Error: No branch set in HEAD.\n");
        return;
    }

    string commitSha = getLatestCommit(branch);

    if (commitSha.empty()) {
        printError("Error: No commits found in the repository.\n");
        return;
    }

    while (!commitSha.empty()) {
        string commitData = readObject(commitSha);
        stringstream ss(commitData);
        string line;

        string tree, parent, author, committer, message;
        while (getline(ss, line) && !line.empty()) {
            if (line.find("tree ") == 0) {
                tree = line.substr(5);
            } else if (line.find("parent ") == 0) {
                parent = line.substr(7);
            } else if (line.find("author ") == 0) {
                author = line.substr(7);
            } else if (line.find("committer ") == 0) {
                committer = line.substr(10);
            }
        }
        getline(ss, message, '\0');
        printOutput("commit " + commitSha + "\n");
        if (!parent.empty()) {
            printOutput("Parent: " + parent + "\n");
        }
        printOutput("Author: " + author + "\n");
        printOutput("Committer: " + committer + "\n\n");
        printOutput(message + "\n\n");

        commitSha = parent;
    }
}

void extractTree(const string &treeHash, const fs::path &path = ".") {
    string treeData = readObject(treeHash);
    stringstream ss(treeData);
    string line;

    while (getline(ss, line)) {
        stringstream ls;
        ls << line;
        string mode, type, hash, name;
        ls >> mode >> type >> hash;
        getline(ls, name);
        name.erase(0, name.find_first_not_of(" ")); // Trim leading spaces

        fs::path objPath = path / name;

        if (type == "tree") {
            fs::create_directory(objPath);
            extractTree(hash, objPath);
        } else if (type == "blob") {
            string content = readObject(hash);
            ofstream file(objPath, ios::binary);
            if (!file) {
                printError("Error creating file " + objPath.string() + "\n");
                continue;
            }
            file << content;
            file.close();
        }
    }
}
void checkout(const string &commitSha) {
    // Read commit object
    string commitData = readObject(commitSha);
    stringstream ss(commitData);
    string line;

    string tree;
    while (getline(ss, line) && !line.empty()) {
        if (line.find("tree ") == 0) {
            tree = line.substr(5);
        }
    }

    if (tree.empty()) {
        printError("Invalid commit object.\n");
        return;
    }

    
    for (const auto &entry : fs::directory_iterator(".")) {
        string filename = entry.path().filename().string();
        if (filename == ".mygit" || filename == "mygit")
            continue;
        if (fs::is_directory(entry.path())) {
            fs::remove_all(entry.path());
        } else {
            fs::remove(entry.path());
        }
    }

    // Extract tree
    extractTree(tree);

    // Update branch reference
    string branch = getCurrentBranch();
    fs::path branchPath = fs::path(MYGIT_DIR) / branch;
    ofstream ref(branchPath, ios::trunc);
    if (!ref) {
        errorExit("Error updating branch reference.");
    }
    ref << commitSha << "\n";
    ref.close();

    printOutput("Checked out commit " + commitSha + "\n");
}

// Function to display usage information
void showUsage(const string &command) {
    if (command == "cat-file") {
        printError("Usage: ./mygit cat-file <flag> <object_sha>\n");
    } else if (command == "ls-tree") {
        printError("Usage: ./mygit ls-tree [--name-only] <tree_sha>\n");
    } else if (command == "add") {
        printError("Usage: ./mygit add <files...>\n");
    } else if (command == "commit") {
        printError("Usage: ./mygit commit [-m \"message\"]\n");
    } else if (command == "checkout") {
        printError("Usage: ./mygit checkout <commit_sha>\n");
    } else if (command == "hash-object") {
        printError("Usage: ./mygit hash-object [-w] <file>\n");
    } else {
        printError("Unknown command: " + command + "\n");
    }
}
void executeCommand(int argc, char *argv[]) {
    if (argc < 2) {
        printError("No command provided.\n");
        exit(1);
    }

    string command = argv[1];

    // Map commands to integer values for switch-case
    int cmd = -1;
    if (command == "cat-file") cmd = 1;
    else if (command == "write-tree") cmd = 2;
    else if (command == "ls-tree") cmd = 3;
    else if (command == "add") cmd = 4;
    else if (command == "commit") cmd = 5;
    else if (command == "log") cmd = 6;
    else if (command == "checkout") cmd = 7;
    else if (command == "hash-object") cmd = 8;
    else if (command == "init") cmd = 9;
    else cmd = 0; 

    switch (cmd) {
        case 1: // cat-file
            if (argc != 4) {
                showUsage("cat-file");
                exit(1);
            }
            catFile(argv[2], argv[3]);
            break;
        case 2: // write-tree
            writeTreeCommand();
            break;
        case 3: // ls-tree
            if (argc != 3 && argc != 4) {
                showUsage("ls-tree");
                exit(1);
            }
            if (argc == 4) {
                lsTree(argv[2], argv[3]);
            } else {
                lsTree("", argv[2]);
            }
            break;
        case 4: // add
            if (argc < 3) {
                showUsage("add");
                exit(1);
            } else {
                vector<string> files(argv + 2, argv + argc);
                add(files);
            }
            break;
        case 5: // commit
            if (argc == 4 && string(argv[2]) == "-m") {
                commit(argv[3]);
            } else if (argc == 2) {
                commit("Default commit message");
            } else {
                showUsage("commit");
                exit(1);
            }
            break;
        case 6: // log
            log();
            break;
        case 7: // checkout
            if (argc != 3) {
                showUsage("checkout");
                exit(1);
            }
            checkout(argv[2]);
            break;
        case 8: // hash-object
            {
                bool writeFlag = false;
                string filePath;
                if (argc == 4 && string(argv[2]) == "-w") {
                    writeFlag = true;
                    filePath = argv[3];
                } else if (argc == 3) {
                    filePath = argv[2];
                } else {
                    showUsage("hash-object");
                    exit(1);
                }
                hashObject(writeFlag, filePath);
            }
            break;
        case 9: // init
            init();
            break;
        default:
            showUsage(command);
            exit(1);
    }
}

int main(int argc, char *argv[]) {
    executeCommand(argc, argv);
    return 0;
}

/*

    Compile with:
    clang++ -std=c++17 -o mygit main.cpp \
    -I/opt/homebrew/opt/openssl@3/include \
    -L/opt/homebrew/opt/openssl@3/lib \
    -lssl -lcrypto -lz

*/
