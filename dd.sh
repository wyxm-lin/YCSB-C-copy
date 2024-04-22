# # 创建一个包含4096 * 1024个字符'a'的输入文件
# printf 'a%.0s' {1..4194304} > input.txt
# # 使用dd命令将input.txt的内容写入到$rbd_file
# rbd_file="/home/linguangming/wyxm/YCSB-C/RBD/simpledb.txt"
# sudo dd if=input.txt of=$rbd_file bs=4K count=1024 oflag=direct

# # 创建一个包含4KB的字符'a'的输入文件
# printf 'a%.0s' {1..4096} > input.txt

# # 使用dd命令将input.txt的内容写入到$rbd_file
# rbd_file="/home/linguangming/wyxm/YCSB-C/RBD/simpledb.txt"
# sudo dd if=input.txt of=$rbd_file bs=4K count=4194304 iflag=fullblock oflag=direct

file="/home/linguangming/wyxm/YCSB-C/DBMS/ddfile.txt"
sudo dd if=/dev/zero of=$file bs=4K count=5000000


# real_disk="/home/linguangming/outcome/AboutRAM/testfile"
# echo "Testing real disk write speed..."
# dd if=/dev/zero of=$real_disk bs=1M count=1024 oflag=direct
# echo "Testing real disk read speed..."
# dd if=$real_disk of=/dev/null bs=1M count=1024 iflag=direct



