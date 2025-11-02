# MyGit

MyGit is a lightweight, custom version control system implemented in C++. It emulates some of the core functionalities of Git, allowing you to track changes, commit snapshots, and manage your project's history with ease.

## Table of Contents

- [Features](#features)
- [Prerequisites](#prerequisites)
- [Installation](#installation)
- [Usage](#usage)
  - [Initializing a Repository](#initializing-a-repository)
  - [Adding Files](#adding-files)
  - [Committing Changes](#committing-changes)
  - [Viewing Commit Logs](#viewing-commit-logs)
  - [Checking Out Commits](#checking-out-commits)
  - [Hashing Objects](#hashing-objects)
  - [Displaying Object Information](#displaying-object-information)
  - [Listing Tree Contents](#listing-tree-contents)
- [Examples](#examples)
- [Project Structure](#project-structure)
- [Contributing](#contributing)
- [License](#license)

## Features

- **Initialize Repository:** Set up a new MyGit repository.
- **Add Files:** Stage files for commit.
- **Commit Changes:** Save snapshots of your project with commit messages.
- **Log History:** View the history of commits.
- **Checkout Commits:** Switch between different commit snapshots.
- **Hash Objects:** Compute SHA-1 hashes of files and objects.
- **Display Object Information:** Inspect the contents and types of stored objects.
- **List Tree Contents:** View the contents of a tree object.

## Prerequisites

Before you begin, ensure you have met the following requirements:

- **Operating System:** macOS, Linux, or Windows.
- **Compiler:** `clang++` or `g++` with C++17 support.
- **Libraries:**
  - **OpenSSL:** For SHA-1 hashing.
  - **zlib:** For data compression.

### Installing Dependencies on macOS (Homebrew)

```bash
brew install openssl@3 zlib
```

### Installing Dependencies on Ubuntu/Linux

```bash
sudo apt-get update
sudo apt-get install libssl-dev zlib1g-dev
```

## Installation

1. **Clone the Repository:**

   ```bash
   git clone https://github.com/yourusername/mygit.git
   cd mygit
   ```

2. **Compile the Program:**

   Use the following command to compile `main.cpp` into the `mygit` executable. Ensure that the OpenSSL and zlib libraries are correctly linked.

   ```bash
   clang++ -std=c++17 -o mygit main.cpp \
   -I/opt/homebrew/opt/openssl@3/include \
   -L/opt/homebrew/opt/openssl@3/lib \
   -lssl -lcrypto -lz
   ```

   **Note:** Adjust the include (`-I`) and library (`-L`) paths based on your system's OpenSSL and zlib installation directories.

3. **Verify Compilation:**

   Ensure that the `mygit` executable has been created.

   ```bash
   ls -l mygit
   ```

## Usage

After installation, you can use `mygit` to manage your projects. Below are the primary commands and their descriptions.

### Initializing a Repository

Initialize a new MyGit repository in your current directory.

```bash
./mygit init
```

**Expected Output:**

```
Initialized empty mygit repository in /path/to/your/project/.mygit
```

### Adding Files

Stage files for the next commit.

```bash
./mygit add <file1> <file2> ...
```

**Example:**

```bash
./mygit add file1.txt file2.txt
```

**Expected Output:**

```
Added file1.txt
Added file2.txt
```

### Committing Changes

Commit staged changes with a commit message.

```bash
./mygit commit -m "Your commit message"
```

**Example:**

```bash
./mygit commit -m "Initial commit"
```

**Expected Output:**

```
Committed as <commit_hash>
```

If no commit message is provided, a default message is used:

```bash
./mygit commit
```

**Expected Output:**

```
Committed as <commit_hash>
```

### Viewing Commit Logs

Display the commit history.

```bash
./mygit log
```

**Expected Output:**

```
commit <commit_hash>
Parent: <parent_commit_hash>
Author: YourName <you@example.com> <timestamp> +0000
Committer: YourName <you@example.com> <timestamp> +0000

Your commit message

...
```

### Checking Out Commits

Switch to a specific commit snapshot.

```bash
./mygit checkout <commit_hash>
```

**Example:**

```bash
./mygit checkout 0ac624624c19a97f8ab357561ab304ce90a54b4b
```

**Expected Output:**

```
Checked out commit 0ac624624c19a97f8ab357561ab304ce90a54b4b
```

### Hashing Objects

Compute the SHA-1 hash of a file. Optionally, write the object to the repository.

```bash
./mygit hash-object [-w] <file>
```

- `-w`: Write the object to the repository.

**Examples:**

- **Compute Hash:**

  ```bash
  ./mygit hash-object file1.txt
  ```

  **Output:**

  ```
  <sha1_hash>
  ```

- **Compute and Write Hash:**

  ```bash
  ./mygit hash-object -w file1.txt
  ```

  **Output:**

  ```
  <sha1_hash>
  Object written to repository.
  ```

### Displaying Object Information

Inspect the contents and type of stored objects.

```bash
./mygit cat-file <flag> <object_sha>
```

**Flags:**

- `-p`: Pretty-print the object’s content.
- `-s`: Print the size of the object.
- `-t`: Print the type of the object (`blob` or `tree`).

**Examples:**

- **Pretty-Print Content:**

  ```bash
  ./mygit cat-file -p <object_sha>
  ```

- **Print Size:**

  ```bash
  ./mygit cat-file -s <object_sha>
  ```

- **Print Type:**

  ```bash
  ./mygit cat-file -t <object_sha>
  ```

### Listing Tree Contents

List the contents of a tree object.

```bash
./mygit ls-tree [--name-only] <tree_sha>
```

- `--name-only`: Display only the filenames.

**Examples:**

- **Detailed Listing:**

  ```bash
  ./mygit ls-tree <tree_sha>
  ```

  **Output:**

  ```
  100644 blob <blob_hash> file1.txt
  100644 blob <blob_hash> file2.txt
  ```

- **Name-Only Listing:**

  ```bash
  ./mygit ls-tree --name-only <tree_sha>
  ```

  **Output:**

  ```
  file1.txt
  file2.txt
  ```

## Examples

Here’s a walkthrough of basic MyGit operations:

1. **Initialize Repository:**

   ```bash
   mkdir mygit_project
   cd mygit_project
   ../mygit init
   ```

2. **Add Files:**

   ```bash
   echo "Hello World" > hello.txt
   ../mygit add hello.txt
   ```

3. **Commit Changes:**

   ```bash
   ../mygit commit -m "Add hello.txt"
   ```

4. **View Commit Logs:**

   ```bash
   ../mygit log
   ```

5. **Make Changes and Commit Again:**

   ```bash
   echo "Hello MyGit" > hello.txt
   ../mygit add hello.txt
   ../mygit commit -m "Update hello.txt"
   ```

6. **Check Out Previous Commit:**

   ```bash
   ../mygit checkout <initial_commit_hash>
   ```

7. **Hash an Object:**

   ```bash
   ../mygit hash-object -w hello.txt
   ```

8. **Display Object Information:**

   ```bash
   ../mygit cat-file -p <object_sha>
   ../mygit cat-file -s <object_sha>
   ../mygit cat-file -t <object_sha>
   ```

9. **List Tree Contents:**

   ```bash
   ../mygit ls-tree <tree_sha>
   ../mygit ls-tree --name-only <tree_sha>
   ```

## Project Structure

```
mygit/
├── main.cpp          # Main source code
├── README.md         # Project documentation
└── .mygit/           # Repository data (created after init)
    ├── HEAD
    ├── index
    ├── objects/
    └── refs/
        └── heads/
            └── master
```

- **main.cpp:** Contains the implementation of MyGit commands and functionalities.
- **.mygit/:** Stores all repository data, including objects, references, and the index.

## Contributing

Contributions are welcome! Please follow these steps:

1. **Fork the Repository:**

   Click the "Fork" button at the top-right corner of this page.

2. **Clone Your Fork:**

   ```bash
   git clone https://github.com/yourusername/mygit.git
   cd mygit
   ```

3. **Create a Branch:**

   ```bash
   git checkout -b feature/YourFeatureName
   ```

4. **Make Changes and Commit:**

   ```bash
   git commit -m "Add Your Feature"
   ```

5. **Push to Your Fork:**

   ```bash
   git push origin feature/YourFeatureName
   ```

6. **Open a Pull Request:**

   Navigate to the original repository and click "New Pull Request".



**Note:** MyGit is a simplified version control system intended for educational and understanding purposes. It does not encompass all features and security measures of production grade systems like Git.

