project('shmit', 'c')
shlagdir = include_directories('shlag/')

btt = library('btt', 'shlag/shlag_btt.c', include_directories: shlagdir)
btt_test = executable('btt_test', 'shlag/tests/btt_test.c', link_with: btt, include_directories : shlagdir)
pcg_example = executable('pcg_example', 'shlag/examples/pcg_simple.c', include_directories : shlagdir)
shitest_example = executable('shitest_example', 'shlag/examples/shitest_example.c', include_directories : shlagdir)

test('run btt_test', btt_test)

# poor tests, but could find crash, infinite loop or something
test('run shitest_example (should fail)', shitest_example, should_fail: true)

max_uint32='4294967295'
args = ['5000', '9898989', max_uint32] # [count, min, max]
test('run pcg_example without specified seed', pcg_example, args : args)
args += ['2137', '2138'] # [seed1, seed2] 
test('run pcg_example with specified seed', pcg_example, args : args)
test('run pcg_example without args (should fail)', pcg_example, should_fail: true)
