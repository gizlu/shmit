project('shmit', 'c', 'cpp')
shlagdir = include_directories('shlag/')

b64test = executable('b64test', 'shlag/tests/b64test.c', include_directories : shlagdir)
pcg_example = executable('pcg_example', 'shlag/examples/pcg_simple.c', include_directories : shlagdir)
shitest_example = executable('shitest_example', 'shlag/examples/shitest_example.c', include_directories : shlagdir)

test('run b64test', b64test)

# poor tests, but could find crash, infinite loop or something
test('run shitest_example (should fail)', shitest_example, should_fail: true)

test('run pcg_example without args (should fail)', pcg_example, should_fail: true)

args = ['5000', '0', '1'] # [count, begin, end]
test('run pcg_example with small range', pcg_example, args : args)

args = ['5000', '9898989', '4294967295'] # [count, begin, end]
test('run pcg_example with big range', pcg_example, args : args)

args = ['5000', '9898989', '4294967295', '2137', '2138'] # [count, begin, end, seed1, seed2]
test('run pcg_example with big nums and specified seed', pcg_example, args : args)

dupa = executable('dupa', 'abyss/dupa.cpp')
