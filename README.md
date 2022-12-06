# sedaman
SEismic DAta MANipulation is a SEGY and SEGD files manipulating library.

# Supported versions
Version    | Read               | Write
---------- | ------------------ | ------------------
SEGY Rev 2 | :heavy_check_mark: | :heavy_check_mark:
SEGY Rev 1 | :heavy_check_mark: | :heavy_check_mark:
SEGY Rev 0 | :heavy_check_mark: | :heavy_check_mark:
SEGD Rev 3 | :heavy_check_mark: | :x:
SEGD Rev 2 | :heavy_check_mark: | :heavy_check_mark:
SEGD Rev 1 | :heavy_check_mark: | :x:

# Prerequsites
Python headers in inlude path for Python library.

# Build C++ library
```
git clone --recurse-submodules https://github.com/andalevor/sedaman
mkdir sedaman/build
cd sedaman/build
cmake .. -DCMAKE_BUILD_TYPE=Release
make sedaman
```
# Build Python library
```
git clone --recurse-submodules https://github.com/andalevor/sedaman
mkdir sedaman/build
cd sedaman/build
cmake .. -DCMAKE_BUILD_TYPE=Release
make pysedaman
```
# Python example
```Python
isegy = pysedaman.ISEGY("stack.sgy")
length = 0.0
h1 = isegy.read_header()
for trace in isegy:
    h2 = trace.header()
    length += math.sqrt((h2.get("CDP_X")-h1.get("CDP_X"))**2+(h2.get("CDP_Y")-h1.get("CDP_Y"))**2)
    h1 = h2

print(length)
```


andalevor@gmail.com
