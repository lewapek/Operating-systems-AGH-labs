echo "\tCreation start"
libtoolize
aclocal
autoconf
autoheader
automake -a

echo "\n\t Configuring..."
./configure --enable-double

echo "\n\t Make"
make

echo "\n\t make install"
sudo make install

