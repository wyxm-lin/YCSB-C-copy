cmake . -DPERF=OFF -DTRANSPORT=infiniband
make -j
sudo ctest


VT0 没有跑通