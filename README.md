## Cloning the repo recursively with all its dependencies 

```
git clone --recursive https://github.com/silhavyj/cpp_starter_project.git
```

## Building the app locally

### Compilation

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
make -C build
```

### Running the app

```
./build/src/myproject
```

### Running tests

```
./build/test/mytest
```

## Running the app in Docker container


### Building a Docker image

```
docker build -t myproject-docker-image .
```

### Creating a Docker container

```
docker run --name myproject myproject-docker-image
```

### Running the app

```
docker start -a myproject
```

## Generating a documentation

```
mkdir -p doc/doxygen
```

```
doxygen doxygen.conf
```