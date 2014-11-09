Raven-tube
==========
Team members:       - Chelsea Bridson
                    - Justin Grothe
                    - Josh Curl
                    
Sources:
http://stackoverflow.com/questions/17015970/how-does-c-compiler-convert-escape-sequence-to-actual-bytes
http://flex.sourceforge.net/manual/Start-Conditions.html

Extra Credit Status:
Debugging mode is implemented with both *index out-of-bounds* and *invalid memory request*

Test Results
==========
- [x] good.01.tube
- [x] good.02.tube
- [x] good.03.tube
- [x] good.04.tube
- [x] good.05.tube
- [x] good.06.tube
- [x] good.07.tube
- [x] good.08.tube
- [x] good.09.tube
- [x] good.10.tube
- [x] good.11.tube
- [x] good.12.tube
- [x] good.13.tube
- [x] good.14.tube
- [x] good.15.tube 
- [x] good.16.tube
- [x] good.17.tube
- [x] good.18.tube
- [x] good.19.tube
- [x] good.999.tube
- [x] fail.01.tube
- [x] fail.02.tube
- [x] fail.03.tube
- [x] fail.04.tube
- [x] fail.05.tube
- [x] fail.06.tube
- [x] fail.07.tube
- [x] fail.08.tube
- [x] fail.09.tube
- [x] fail.10.tube

Required Error Messages 
====================

- [x] ERROR(line #): unknown token '@'
- [x] ERROR(line #): syntax error
- [x] ERROR(line #): unknown variable 'var_name'
- [x] ERROR(line #): redeclaration of variable 'var_name' 
- [x] ERROR(line #): 'break' command used outside of any loop
- [x] ERROR(line #): types do not match for assignment (lhs='type1', rhs='type2')
- [x] ERROR(line #): types do not match for relationship operator (lhs='type1', rhs='type2')
- [x] ERROR(line #): cannot use type 'type' in mathematical expressions
          (note that the above error should be triggered for all of +, -, *, /, %, &&, ||, !, +=, -=, *=, /=, and %=)
- [x] ERROR(line #): condition for if statements must evaluate to type int
- [x] ERROR(line #): condition for while statements must evaluate to type int
- [x] ERROR(line #): cannot use type 'type' as an argument to random 
- [x] ERROR(line #): cannot index into a non-array type
- [x] ERROR(line #): array indices must be of type int
- [x] ERROR(line #): array methods cannot be run on type 'type'
          size() and resize() are the two legal array methods; the identifier before them must represent an array variable.
- [x] ERROR(line #): array size() method does not take any arguments."
- [x] ERROR(line #): array resize() method takes exactly one (int) argument.
- [x] ERROR(line #): array resize() method argument must be of type int. 
