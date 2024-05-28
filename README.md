# AlChemy
This repository contains code and scripts to run `AlChemy` as originally designed by Walter Fontana and Leo Buss. The original code base can be found here: https://sites.santafe.edu/~walter/AlChemy/software.html 

The C++ code in this repository differs from the original zip file, it has been modified to compile on modern machines.

As with the original code base, this code is for people who are acustomed to using software "in the wild."
There are not nearly as many comments as there should be, and some things can only be understoof by emulating the
thought patterns of the original authors. We apologize in advance for that.

## Outline
This repository contains
- AlChemy (C++) code that compiles
- Python Scripts (PyAlChemy) to faciliate the running and analysis of the original C++ executable
- A Dockerfile to compile the code in a container and install relevant python dependencies.
- Python scripts needed to recreate data from Mathis et. al. 2024.


## Compiling
We have only been able to build `AlChemy` on Unix systems.

You should be able to navigate directly to `LambdaReactor` and run `make all`. If this exits without error you should have an executable `ALCHEMY` in that directory. Test it by running `./ALCHEMY` which will read the default input file `alchemy.inp` and will output some files from the run. If this exits with an error we recommend you look at the dockerizer version below. 

# PyAlChemy
We've developed some python scripts to wrap AlChemy and call it programmatically. These can be used after you compile the executable. You'll need to modify the `LAMBDA_PATH` variable towards the top of the file `PyAlChemy/PyAlChemy.py` (approximately line 8).

You can get an idea of how to use the Python scripts by inspecting and running the example included at the bottom of `PyAlChemy/PyAlchemy.py`

## Dockerize container
If you have docker installed you can inspect `Dockerfile` to understand the container. Then you can build and run it using these commands
``` 
> docker build -t alchemy .
> docker container run -it alchemy bash
```

# Mathis et. al. 2024 Results

Within `PyAlChemy` there are scripts to reproduce the results from the Mathis et al 2024 paper (Return to AlChemy). The key results are split into folders. There is a script to run the simulation, perform the analysis, and a Jupyter notebook to reproduce the results. The dependencies beyond AlChemy are relatively standard, and can all be installed via pip. The plotting was done with `R`, but could be easily translated into pandas by the interested reader.