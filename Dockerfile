##
# docker build -t  mmstreamer_docker_image .
# docker run -it -u $(id -u) --rm -w="/mmstreamer" -v $(pwd):/mmstreamer mmstreamer_docker_image:latest /bin/bash
##

# Download base image ubuntu 16.04
FROM ubuntu:16.04

# Define labels (maintainer and description)
LABEL maintainer="Boubacar DIENE <boubacar.diene@gmail.com>"
LABEL description="Docker image to build mmstreamer project"

# Install required packages from ubuntu repository
RUN apt-get update && apt-get -y upgrade && apt-get install -y \
        build-essential \
        libgl1-mesa-glx \
        libjpeg8-dev \
        libpng16-dev \
        libv4l-dev \
        libx11-dev \
        libxext-dev \
        openssh-server \
        ruby \
        tree \
        valgrind \
        vim \
        zlib1g-dev && \
    apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

# Create symlinks
RUN cd /usr/lib/ && ln -s x86_64-linux-gnu/mesa/libGL.so.1 libGL.so && cd -

# Install ceedling
RUN gem install ceedling

# Expose SSH port
EXPOSE 22
CMD ["/usr/sbin/sshd", "-D"]
