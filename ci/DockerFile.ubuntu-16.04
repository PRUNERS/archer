FROM ubuntu:16.04
RUN apt-get update

# Set the locale
RUN locale-gen en_US.UTF-8
ENV LANG en_US.UTF-8
ENV LANGUAGE en_US:en
ENV LC_ALL en_US.UTF-8

# Install dependencies
ENV DEBIAN_FRONTEND noninteractive
RUN apt-get update
RUN apt-get install -qq build-essential cmake ninja-build

# Grab the checked out source
# RUN mkdir -p /workdir
# WORKDIR /workdir
# COPY . /workdir
# RUN make
# RUN make pkg
