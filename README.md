# C Suite

A small, self‑contained collection of **useful, no‑nonsense data structures written in C**.

The goal of C Suite is to provide practical building blocks that are:

* **Simple** to modify/integrate
* Relatively **Well‑documented**
* Relatively **Portable** across compilers

No dependencies — just independent `.c` + `.h` pairs you can drop into your project.

---

### Included Data Structures

* **Vec** — Dynamic, heap‑allocated array
* **Vstr** — Dynamic, heap‑allocated string
* **LList** — Intrusive doubly‑linked list
* **Arena** — Arena allocator
* **FixedBuffer** — Fixed‑size buffer allocator
* **Queue** — Single‑producer / single‑consumer lock‑free queue
* **Hashmap** — Linked‑list‑based hashmap

---

### Documentation

Each header is **heavily commented** with Doxygen-style docs, and should be sufficent.  
Additionally the `tests/` directory has tests for everything and they can be used as reference, although the quality is lower there.

---

### Build & Compatibility

* Compiles out of the box with **any C99‑compliant compiler**
* **MSVC compatible**, with one exception:

  * `Queue` uses atomic operations. Those come with C99 and i don't think MSVC has them

No external libraries are required.
