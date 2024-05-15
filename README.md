# AlChemy

# Original Code

## Outline

## Compiling
We have only been able to build `AlChemy` on Unix systems.

You should be able to navigate directly to `LambdaReactor` and run `make all`. If this exits without error you should have an executable `ALCHEMY` in that directory. Test it by running `./ALCHEMY` which will read the default input file `alchemy.inp` and will output some files from the run.

# PyAlChemy
We've developed some python scripts to wrap AlChemy and call it programmatically. hese can be used after you compile the executable. You'll need to modify the `LAMBDA_PATH` variable towards the top of the file `PyAlChemy/PyAlChemy.py` (approximately line 10).

# Mathis et. al. 2024 Results

Within PyAlChemy there are scripts to reproduce the results from the Mathis et al 2024 paper (Return to AlChemy). The key results are split into folders. There is a script to run the simulation, perform the analysis, and a Jupyter notebook to reproduce the results. The dependencies beyond AlChemy are relatively standard, and can all be installed via pip. The plotting was done with `R`, but could be easily translated into pandas by the interested reader. 