#include <mthread_common_helpers.h>
#include <mthread_scheduler_helpers.h>

#define Check_val(val)                                                         \
  {                                                                            \
    assert(res == (val));                                                      \
    if (res != (val)) {                                                        \
      return 1;                                                                \
    }                                                                          \
  }                                                                            \
  (void)(0)

int main() {
  {
    mthread_list_item_t *res;
    res = remove_head_in_list(nullptr);
    Check_val(nullptr);
  }
  {
    mthread_list_t _list;
    mthread_list_item_t *res;
    res = remove_head_in_list(&_list);
    Check_val(nullptr);
  }
  {
    mthread_list_t _list;
    mthread_list_item_t item;
    mthread_list_item_t *res;
    insert_tail_in_list(&item, &_list);

    res = remove_head_in_list(&_list);
    Check_val(&item);
  }
  {
    mthread_list_t _list;
    mthread_list_item_t item_1;
    mthread_list_item_t item_2;
    mthread_list_item_t *res;
    insert_tail_in_list(&item_1, &_list);
    insert_tail_in_list(&item_2, &_list);

    res = remove_head_in_list(&_list);
    Check_val(&item_1);
    res = remove_head_in_list(&_list);
    Check_val(&item_2);
  }
  {
    mthread_list_item_t item;
    mthread_list_t _list;
    insert_tail_in_list(nullptr, &_list);
    insert_tail_in_list(&item, &_list);
  }
  return 0;
}