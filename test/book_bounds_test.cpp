#include "book.h"
#include <cassert>
#include <cstdio>
int main() {
    Book book;book.AppendPage();book.AppendPage();book.AppendPage();
    book.SetPosition(-5);assert(book.GetPosition()==0);
    book.SetPosition(65000);assert(book.GetPosition()==2);
    assert(book.GetPage()!=NULL);
    book.SetPosition(1);assert(book.GetPosition()==1);
    std::puts("PASS: negative positions and stale bookmarks stay within loaded pages");
}
