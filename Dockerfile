FROM pgrouting/pgrouting:13-3.1-3.1.3

ARG CMAKE_VERSION=3.22.2
ARG H3_VERSION=3.7.2
ARG CPUS=4

# dependencies
RUN apt-get update
RUN apt-get install -y git wget gcc build-essential libssl-dev clang-format cmake-curses-gui lcov doxygen libtool postgresql-server-dev-13

# install cmake
RUN wget https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}.tar.gz
RUN tar xzf cmake-${CMAKE_VERSION}.tar.gz
WORKDIR cmake-${CMAKE_VERSION}
RUN ./bootstrap
RUN make
RUN make install
WORKDIR ..

# install h3 binding
RUN mkdir h3
WORKDIR h3
RUN wget https://github.com/uber/h3/archive/refs/tags/v${H3_VERSION}.tar.gz
RUN tar xf v${H3_VERSION}.tar.gz
RUN mkdir build
WORKDIR build
RUN cmake -DCMAKE_C_FLAGS=-fPIC ../h3-${H3_VERSION}
RUN make -j $CPUS
RUN make install
WORKDIR ../../

# install pgh3
RUN git clone https://github.com/Anagraph/pgh3
WORKDIR pgh3
RUN git checkout ca5307f
RUN make -j ${CPUS}
RUN make install
WORKDIR ..