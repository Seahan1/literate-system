#include <unity.h>
#include <string.h>
#include "core/hub_proto.h"

void setUp(void) {}
void tearDown(void) {}

static void test_notes_frame_parses_titles(void) {
  const char* json = "{\"v\":1,\"notes\":{\"titles\":[\"First\",\"Second\"]}}";
  notes_rec_t notes;
  memset(&notes, 0, sizeof(notes));
  bool had = false;
  TEST_ASSERT_TRUE(hub_parse_notes(json, strlen(json), &notes, &had));
  TEST_ASSERT_TRUE(had);
  TEST_ASSERT_EQUAL_UINT8(2, notes.count);
  TEST_ASSERT_EQUAL_STRING("First", notes.titles[0]);
  TEST_ASSERT_EQUAL_STRING("Second", notes.titles[1]);
}

static void test_todo_frame_parses_today_and_overdue(void) {
  const char* json = "{\"v\":1,\"todo\":{\"today\":[\"Call\"],\"overdue\":[\"Pay bill\"]}}";
  todo_rec_t todo;
  memset(&todo, 0, sizeof(todo));
  bool had = false;
  TEST_ASSERT_TRUE(hub_parse_todo(json, strlen(json), &todo, &had));
  TEST_ASSERT_TRUE(had);
  TEST_ASSERT_EQUAL_UINT8(1, todo.today_count);
  TEST_ASSERT_EQUAL_UINT8(1, todo.overdue_count);
  TEST_ASSERT_EQUAL_STRING("Call", todo.today[0]);
  TEST_ASSERT_EQUAL_STRING("Pay bill", todo.overdue[0]);
}

static void test_organizer_frames_require_title_arrays(void) {
  const char* notes_json = "{\"v\":1,\"notes\":{}}";
  const char* todo_json = "{\"v\":1,\"todo\":{\"today\":[]}}";
  notes_rec_t notes;
  todo_rec_t todo;
  bool had_notes = false;
  bool had_todo = false;
  TEST_ASSERT_FALSE(hub_parse_notes(notes_json, strlen(notes_json), &notes, &had_notes));
  TEST_ASSERT_FALSE(hub_parse_todo(todo_json, strlen(todo_json), &todo, &had_todo));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_notes_frame_parses_titles);
  RUN_TEST(test_todo_frame_parses_today_and_overdue);
  RUN_TEST(test_organizer_frames_require_title_arrays);
  return UNITY_END();
}
