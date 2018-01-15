echo "\tCreation start"
libtoolize
aclocal
autoconf
autoheader
automake -a

echo "\n\t Configuring..."
./configure --with-block-size=16 --with-allocator=MAX_ALLOC_STRATEGY

echo "\n\t Make"
make

echo "\n\t make install"
sudo make install

