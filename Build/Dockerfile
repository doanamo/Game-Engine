# Warning: Making changes to this file requires triggering image rebuild on Docker Hub!

# Specify base image.
FROM amd64/ubuntu:impish

# Install core dependencies.
RUN apt-get update
RUN apt-get install -y apt-transport-https ca-certificates gnupg software-properties-common wget

# Enable repository with more up-to-date tools.
RUN apt-add-repository universe
RUN apt-get update

# Add repository for latest CMake.
RUN wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | apt-key add -
RUN apt-add-repository 'deb https://apt.kitware.com/ubuntu/ focal main'
RUN apt-get update

# Install developer dependencies.
RUN apt-get install -y git cmake g++-multilib clang
RUN apt-get install -y xorg-dev libgl1-mesa-dev
