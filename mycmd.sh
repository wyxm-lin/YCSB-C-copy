./ycsbc -db real_ceph -threads 1 -P workloads/myworkload.spec > print$(date +%Y%m%d%H%M%S).txt
./ycsbc -db ceph -threads 2 -P workloads/myworkload.spec > print.txt
./ycsbc -db xihu -threads 1 -P workloads/myworkload.spec > print$(date +%Y%m%d%H%M%S).txt
sudo ./ycsbc -db xihu -threads 1 -P workloads/myworkload.spec > print.txt
sudo ./ycsbc -db xihu -threads 1 -P workloads/myworkload.spec

./ycsbc -db basic -threads 1 -P workloads/workloada.spec
sudo apt-get install libtbb-dev
make
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib


# 原网址
https://github.com/basicthinker/YCSB-C.git 