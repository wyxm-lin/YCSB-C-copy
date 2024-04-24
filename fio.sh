fio --name=mytest --ioengine=libaio --rw=read --bs=4k --numjobs=1 --size=40G --runtime=60 --time_based --direct=1
fio --name=mytest --filename=/home/linguangming/wyxm/YCSB-C/DBMS/db.txt --ioengine=libaio --rw=randrw --bs=4k --numjobs=1 --size=40G --direct=1
# 写操作
dd if=/dev/zero of=/home/linguangming/wyxm/YCSB-C/DBMS/db.txt bs=4k count=10485760 oflag=direct
# 读操作
dd if=/home/linguangming/wyxm/YCSB-C/DBMS/db.txt of=/dev/null bs=4k iflag=direct

fio --name=mytest --ioengine=rbd --rw=randwrite --bs=4k --numjobs=1 --size=1G --rados-namespace=fio --rados-pool=rbd --rados-uid=admin --rados-locator=lun1 --runtime=10m --time_based
fio --name=mytest --ioengine=rbd --rw=randwrite --bs=4k --numjobs=1 --size=1G --pool=MyTest --rados-uid=admin --rados-locator=MyTest --runtime=10m --time_based
fio --name=ThisIsRBDFio --ioengine=rbd --rw=randwrite --bs=4k --iodepth=16 --runtime=5m --numjobs=2 --pool=MyTest --rbdname=ceph-rbd-image1.img 
(fio测试合理)
fio --name=ThisIsRBDFio --ioengine=rbd --rw=randwrite --bs=4k --iodepth=16 --size=30G --numjobs=2 --pool=MyTest --rbdname=ceph-rbd-image1.img --direct=1