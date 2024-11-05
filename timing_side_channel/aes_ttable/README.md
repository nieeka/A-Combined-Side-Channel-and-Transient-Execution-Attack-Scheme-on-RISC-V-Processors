wget https://ftp.openssl.org/source/old/1.0.1/openssl-1.0.1e.tar.gz
tar -xzf openssl-1.0.1e.tar.gz
cd openssl-1.0.1e
./config shared
make 

library path:
`export LD_LIBRARY_PATH=$HOME/openssl-1.0.1e:$LD_LIBRARY_PATH`

`make; ./spy-Thead` or `make; ./spy-other`
