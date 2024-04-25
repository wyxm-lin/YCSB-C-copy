```
make clean
make
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib
./ycsbc -db real_ceph -threads 1 -P workloads/myworkload.spec > print$(date +%Y%m%d%H%M%S).txt

# 解释：real_ceph是db的名称 
# workloads/myworkload是负载 
# threads 1是1个线程

```