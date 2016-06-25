#ifndef AHO_CORASICK_TRIE_H
#define AHO_CORASICK_TRIE_H

// Copyright (C) 2012 Jeff Donner
//
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use, copy,
// modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
// BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// (MIT 'expat' license)

#include <algorithm>
#include <vector>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <deque>
#include <memory>

typedef uint8_t AC_CHAR_TYPE;

typedef int32_t PayloadT;
class AhoCorasickTrie;

struct Node {
   typedef int Index;

   Node(PayloadT payload = -1)
   : ifailure_state(0)
   , payload(payload)
   {}

   typedef std::pair<AC_CHAR_TYPE, Index> Child;
   typedef std::vector<Child> Children;

   Index child_at(AC_CHAR_TYPE c) const {
      Children::const_iterator child = std::lower_bound(
              children.begin(), children.end(), Child(c, -1));
      if (child == children.end() || child->first != c)
          // since these are indices, 0 is valid, so invalid is < 0
          return -1;
      return child->second;
   }

   void set_child(AC_CHAR_TYPE c, Index idx) {
      assert(idx);
      Children::iterator child = std::lower_bound(
              children.begin(), children.end(), Child(c, -1));
      if (child != children.end() && child->first == c) {
          child->second = idx;
          return;
      }
      children.insert(child, Child(c, idx));
   }

   const Children& get_children() const {
      return children;
   }

   Index ifailure_state;
   PayloadT payload;

private:
   Children children;
};

std::ostream& operator<<(std::ostream& os, Node const& node);

class FrozenTrie;
typedef std::deque<Node> Nodes;

class AhoCorasickTrie {
public:
   typedef Node::Index Index;
   typedef std::vector<AC_CHAR_TYPE> Chars;
   typedef std::vector<Chars> Strings;

public:
   AhoCorasickTrie();
   ~AhoCorasickTrie();

   void add_string(char const* s, size_t n, PayloadT payload = -1);

   PayloadT find_short(char const* s, size_t n,
                       int* inout_start,
                       int* out_end) const;

   PayloadT find_longest(char const* s, size_t n,
                         int* inout_start,
                         int* out_end) const;

   // Only makes fail links but, I'm hitching on the idea from regexps.
   // You never need to use this, it's done automatically, it's just here
   // in case you want manual control.
   void compile();

   void print() const;

   int contains(char const*, size_t n) const;

   int num_keys() const;

   int num_nodes() const;

   int num_total_children() const;

   /// Returns either a valid ptr (including 0) or, -1 cast as a ptr.
   PayloadT get_payload(char const* s, size_t n) const;


private:
   static bool is_valid(Index ichild);
   void assert_compiled() const;

   /// Does the actual 'compilation', ie builds the failure links
   void make_failure_links();

   Index child_at(Index i, AC_CHAR_TYPE a) const;

   Node::Index add_node();

   // root is at 0 of course.
   // <Node>s are stored by value so, it's expensive when you're
   // building the tree (but still O(n)), but, it saves some bytes in
   // pointers (esp. on 64 bit machines) and enhances continuity (ie
   // prefetchability of cache)). At least, that's the author's
   // untested reason for doing it this way.
   Nodes nodes;
   std::deque<unsigned short> lengths;

   std::unique_ptr<FrozenTrie> frozen;
};

// for debugging
std::ostream& operator<<(std::ostream& os, AhoCorasickTrie::Chars const& text);

std::ostream& operator<<(std::ostream& os, AhoCorasickTrie::Strings const& texts);

// Utf8CodePoints builds a mapping from utf-8 byte index to related codepoint
// index. Use it to convert Trie utf-8 byte offsets into caller string offsets.
class Utf8CodePoints {
public:
    Utf8CodePoints();

    void create(const char* s, size_t n);
    int32_t get_codepoint_index(int byte_index) const;

private:
    std::vector<int32_t> indices;
};

#endif // AHO_CORASICK_TRIE_H
