## Setting up VM

In **web console**:

```
sudo usermod -a -G ssh sm3421
sudo usermod -a -G sudo sm3421
```

Then, add the VM's name to `~/.zshrc` for convenience via, e.g., 

```
echo "export KLEE3=sm3421@cloud-vm-43-235.doc.ic.ac.uk" >> ~/.zshrc
```

```sh
ssh $KLEE<N>
```

---


```sh
cd home
sudo mkdir sm3421
```

```sh
cd sm3421
sudo cp ../ubuntu/.bashrc .
source .bashrc
sudo chown sm3421 .
sudo update-alternatives --config editor # Then select 2
```

```sh
ssh-keygen -t ed25519 -C <MY_EMAIL> # Press enter till done
```

```
cat ~/.ssh/id_ed25519.pub
```

Copy result of `cat` to clipboard, then go to https://github.com/settings/keys, click *"New SSH Key"* and paste it in.

```sh
git clone git@github.com:sreeshmaheshwar/klee.git
```

```
cd klee
git checkout delay-simp # e.g. stack-basic-vs-lcp-pp
```

Add this all to a script, `cat > install.sh` then `bash install.sh`, will need to spam Y.

```sh
sudo apt install libunwind-dev
sudo apt-get install google-perftools libgoogle-perftools-dev # Didn't work
sudo apt-get install build-essential cmake curl file g++-multilib gcc-multilib git libcap-dev libgoogle-perftools-dev libncurses5-dev libsqlite3-dev libtcmalloc-minimal4 python3-pip unzip graphviz
sudo pip3 install lit wllvm
sudo apt-get install python3-tabulate
cd ..
sudo apt-get install wget
wget https://apt.llvm.org/llvm.sh
chmod +x llvm.sh
sudo ./llvm.sh 13 all
```

```sh
git clone https://github.com/klee/klee-uclibc.git
cd klee-uclibc
./configure --make-llvm-lib --with-cc clang-13 --with-llvm-config llvm-config-13
make -j2
cd ..
mv klee-uclibc ..
```

```sh
# sudo apt install libunwind-dev
# sudo apt-get install google-perftools libgoogle-perftools-dev # Didn't work
mkdir build # NEED TO REDO THIS EACH TIME
cd build
sudo chown sm3421 ~/.bashrc
echo "alias g=git" >> ~/.bashrc # For using g for git. 
source ~/.bashrc
```

---

**NOTE**: If multiple build steps are required (e.g. due to the `perftools` issue), you need to `cd ..`, `rm -rf build`, `mkdir build`, `cd build` each time, use

```sh
cd ~/klee && rm -rf build && mkdir build && cd build && cmake -DENABLE_SOLVER_STP=OFF -DENABLE_POSIX_RUNTIME=ON -DKLEE_UCLIBC_PATH=../../klee-uclibc/ -DENABLE_KLEE_ASSERTS=OFF -DCMAKE_BUILD_TYPE=Release -DKLEE_RUNTIME_BUILD_TYPE=Release .. && make
```

---

```sh
sudo apt install libunwind-dev
sudo apt-get install google-perftools libgoogle-perftools-dev # Didn't work
cmake -DENABLE_SOLVER_STP=OFF -DENABLE_POSIX_RUNTIME=ON -DKLEE_UCLIBC_PATH=../../klee-uclibc/ -DENABLE_KLEE_ASSERTS=OFF -DCMAKE_BUILD_TYPE=Release -DKLEE_RUNTIME_BUILD_TYPE=Release ..
make # Takes a while!
```

```sh
cd
wget https://raw.githubusercontent.com/coreutils/coreutils/master/scripts/build-older-versions/coreutils-6.10-on-glibc-2.28.diff
wget https://ftp.gnu.org/gnu/coreutils/coreutils-6.10.tar.gz
tar -xvzf coreutils-6.10.tar.gz
cd coreutils-6.10
patch -p1 < ../coreutils-6.10-on-glibc-2.28.diff
mkdir obj-llvm
cd obj-llvm
pip install --upgrade wllvm
export LLVM_COMPILER=clang
sudo ln /usr/bin/clang-13 /usr/bin/clang # May not work - that's OK
CC=wllvm ../configure --disable-nls CFLAGS="-g -O1 -Xclang -disable-llvm-passes -D__NO_STRING_INLINES  -D_FORTIFY_SOURCE=0 -U__OPTIMIZE__"
make
cd src
ls # Bunch of files!
sudo ln /usr/bin/llvm-link-13 /usr/bin/llvm-link # Only if needed.
find . -executable -type f | xargs -I '{}' extract-bc '{}' # Error is OK.
# /home/sm3421/klee/build/bin/klee --libc=uclibc --posix-runtime ./cat.bc --version # Test it out!
```

---

For reconfiguring existing VM,

```sh
cp ~/coreutils-9.0/obj-llvm/src/test.env .
cd ..
git clone git@github.com:sreeshmaheshwar/klee-test.git
cd klee-test
# Checkout branch
```

---

Otherwise, in `/home/sm3421/coreutils-9.0/obj-llvm/src`

```sh
cd ..
cp -r <LINK_TO_EXPERIMENT_DIR> klee-test # Move experiment directoy here
cd klee-test
# CHECKOUT BRANCH!
echo "export KLEE_USERNAME=sm3421" >> ~/.bashrc # TODO: Need to change name!
source ~/.bashrc
```

--- 

**ENTER** normal terminal. (**TODO:** Can probably `wget` these instead)

```sh
scp ~/testing-env.sh $KLEE<N>:/home/sm3421/coreutils-6.10/obj-llvm/src
scp ~/sandbox.tgz $KLEE<N>:/home/sm3421 
ssh-copy-id $KLEE<N>
```

**RETURN** to VM.

---

```sh
cd ../src && env -i /bin/bash -c '(source testing-env.sh; env >test.env)'
cd ../klee-test
bash setup.sh # On newer branches; enter screen prior.
python3 runklee.py
```

## Building Z3 from Source

**From normal terminal:*

```sh
scp ~/z3-z3-4.13.0.tar.gz $KLEE<N>:/home/sm3421
```

**From KLEE-N:**

```sh
tar -xvzf z3-z3-4.13.0.tar.gz && cd z3-z3-4.13.0 && python3 scripts/mk_make.py && cd build && make # Takes a while.
```

or in steps:

```sh
cd z3-z3-4.13.0
```

```sh
python3 scripts/mk_make.py
```

```sh
cd build && make # Takes a while.
```

---

**If Z3 already installed:**

Now, it is immensely painful, but you need to remove everything z3 related from /usr/include and also the shared library:

```sh
sudo apt-get --purge z3
sudo make install
sudo make uninstall
ls -lt /usr/include/z3* # Should be empty.
sudo rm -rf /usr/lib/x86_64-linux-gnu/libz3.so*
```

---

```sh
sudo make install
ls -lt /usr/include/z3*
ls -lt /usr/lib/x86_64-linux-gnu/libz3.so*
```

---

**If KLEE already built:**

```sh
cd && cd klee && rm -rf build && mkdir build && cd build && cmake -DENABLE_SOLVER_STP=OFF -DENABLE_POSIX_RUNTIME=ON -DKLEE_UCLIBC_PATH=../../klee-uclibc/ -DENABLE_KLEE_ASSERTS=OFF -DCMAKE_BUILD_TYPE=Release -DKLEE_RUNTIME_BUILD_TYPE=Release ..
```
