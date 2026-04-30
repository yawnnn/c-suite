# C Suite

A small, self‑contained collection of **useful, fundamental data structures written in C**.

The goal of C Suite is to provide practical building blocks that are:

* **Simple** to modify/integrate
* **Well‑documented API**
* **Portable**, see [Build & Compatibility](#Build-&-Compatibility)

No dependencies — just independent `.c` + `.h` pairs you can drop into your project.

---

### Included Data Structures

* **Vec** — Dynamic, heap‑allocated array
* **VStr** — Dynamic, heap‑allocated string
* **LList** — Intrusive doubly‑linked list
* **Arena** — Arena allocator
* **FixedBuffer** — Fixed‑size buffer allocator
* **Queue** — Single‑producer / single‑consumer lock‑free queue
* **Hashmap** — Linked‑list‑based hashmap

---

### Documentation

Each header is **heavily commented** with Doxygen-style docs, and should be sufficent.  
Additionally you can check out the `tests/` and `benches/` directory for examples, although they're not particularly curated for that purpose.

---

### Build & Compatibility

* All data structures (except `Queue`) are **portable C99**
* Should compile with any standard C compiler (GCC, Clang, MSVC)
* No external dependencies for the core library
* Tests and benches have some dependencies

#### Queue (SPSC)

* Requires **C11 atomics** (`<stdatomic.h>`), and on MSVC `/experimental:c11atomics`

