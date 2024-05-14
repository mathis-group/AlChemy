# Specify your docker image 
# (this is essentially the operating system you're starting with)
FROM ubuntu:22.04

# Install gcc-11 and make
RUN apt-get update
RUN apt-get install -y gcc-11
RUN apt-get install make 

# Let's install Python
# Edit the parameters below to specify python version
RUN apt-get update && apt-get install -y python3.11
# This makes sure that CMD [python] does what you think
RUN apt-get install -y python-is-python3

WORKDIR /home/AlChemy
COPY . .
WORKDIR /home/AlChemy/LambdaReactor
RUN make clean
RUN make CC=gcc-11
