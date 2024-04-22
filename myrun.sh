for i in {1..5}
do 
    echo "Runing iteration $i"
    ./ycsbc -db ceph -threads 1 -P workloads/myworkload.spec > "print_ceph$i.txt"
done

