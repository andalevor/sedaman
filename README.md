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

# Build C++ library
```
git clone https://github.com/andalevor/sedaman
mkdir sedaman/build
cd sedaman/build
cmake .. -DCMAKE_BUILD_TYPE=Release
make sedaman
```
# Build Python library
```
git clone https://github.com/andalevor/sedaman
mkdir sedaman/build
cd sedaman/build
cmake .. -DCMAKE_BUILD_TYPE=Release
make pysedaman
```

andalevor@gmail.com
