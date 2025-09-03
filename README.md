- simple hashmap 
- robinhood open addressing w/ linear probing
- stores values

## Project Structure
- `src/`: source code for chmap; these are the important bits if you want to use it!
- `test/`: unit tests
- `unity/`: source code for the unit testing framework

## Unit Testing Structure
- Tests are contained in `test/`.
- Each test starts with `test_`.
- One file should adequately test one piece of functionality.
- Each file will be compiled into its own executable. This is to isolate program-crashing errors.
- The files are then ran, and the output is piped into .txt files.
- The files are then concatenated into the terminal to view.

## Makefile
- `make` by default will run unit tests