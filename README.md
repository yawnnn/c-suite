# C Vector and String
A version of the dynamic structures Vec and String that you would find in several languages, implemented in C.

To clarify, this implementation is geared towards being used in an existing C codebase (which is what i'll do), and as such, i've made design decision with that in mind.\
As an example Vstr (aka String) is null-terminated, even though internally I keep track of it's length, 'cause eventually you'll need to pass it to a function that expects a c-style string