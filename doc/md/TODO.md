# tRIBS TODO

### Todo
- [ ] Refactor tResample::doIt, code is unclear and possible bottle neck.
- [ ] Review/remove remaining user run flags
- [ ] Check if GAUGEBASENAME: Rain Gauge data BASE name (*.mdf) is needed, this is superseded by the .sdf file.
- [ ] Update flow for reading in dynamic LU grids--this is a bottleneck in terms of speed
- [ ] Remove InputTime from .in file and tRIBS processes as it should always be set to zero.
- [ ] Create parameter input file for snowpack.cpp
- [ ] Consider flexible approach for specifying outputs, I.E. could we make it so that you could pass in attribute related to a node and have them returned in the dynamic and integrated files. Could also be nice for input parameters, i.e. make list of hard coded parameters and expose as inputs.
### Finished
- [x] Finalize updated benchmarks
- [x] Remove invariant .pixel files--all relevant information can be written to the time integrated variable
- [x] Merged tSnowIntercept with tSnowPack
- [x] Fixed larger memory leaks when running in parallel
- [x] Update .in file and inputs, including moving command line flags and unnecessary options in .in files
- [x] Fix compiler warnings (sprintf to sprintnf, etc ...)
- [x] Update compiler flags and release version for optimized performance


Return to [README](../../README.md)
