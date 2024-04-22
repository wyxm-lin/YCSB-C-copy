./ycsbc -db ceph -threads 1 -P workloads/myworkload.spec > print.txt

sudo apt-get install libtbb-dev
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib
make

# 原网址
https://github.com/basicthinker/YCSB-C.git 