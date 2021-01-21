##
# docker build -t  mmstreamer-image ci/
# docker run --privileged -it -u $(id -u) --rm -v $(pwd):/workdir mmstreamer-image:latest
##

# Download base image ubuntu 16.04
FROM ubuntu:16.04

# Define labels (maintainer and description)
LABEL maintainer="Boubacar DIENE <boubacar.diene@gmail.com>"
LABEL description="Docker image to build mmstreamer project"

# Install required packages from ubuntu repository
RUN apt-get update && apt-get -y upgrade && apt-get install -y \
        build-essential \
        clang-format \
        cppcheck \
        ctags \
        git \
        glmark2 \
        libgl1-mesa-dev \
        libgl1-mesa-glx \
        libjpeg8-dev \
        libpng16-dev \
        libv4l-dev \
        libx11-dev \
        libxext-dev \
        lshw \
        openssh-server \
        python3-pip \
        ruby \
        strace \
        tree \
        valgrind \
        vim \
        x11-apps \
        zlib1g-dev && \
    apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

# Create symlinks
RUN cd /usr/lib/ && ln -s x86_64-linux-gnu/mesa/libGL.so.1 libGL.so && cd -

# Install ceedling and requirements
RUN gem install ceedling -v 0.29.1
RUN pip3 install gcovr

# Add user
RUN useradd -ms /bin/bash mmstreamer
USER mmstreamer
WORKDIR /workdir

# Install vim plugins
RUN mkdir -p /home/mmstreamer/.vim/autoload /home/mmstreamer/.vim/bundle \
    && wget https://tpo.pe/pathogen.vim -P /home/mmstreamer/.vim/autoload/ \
    && git clone https://github.com/kien/ctrlp.vim.git /home/mmstreamer/.vim/bundle/ctrlp.vim \
    && git clone https://github.com/scrooloose/nerdtree.git /home/mmstreamer/.vim/bundle/nerdtree \
    && git clone https://github.com/kana/vim-operator-user.git /home/mmstreamer/.vim/bundle/vim-operator-user \
    && git clone https://github.com/rhysd/vim-clang-format.git /home/mmstreamer/.vim/bundle/vim-clang-format

COPY .vimrc /home/mmstreamer/

# Entry point
ENTRYPOINT ["/bin/bash"]
