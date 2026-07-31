/* USER CODE BEGIN media_proto_tests */
#include <unity.h>
#include <string.h>
#include "core/hub_proto.h"

void setUp(void) {}
void tearDown(void) {}

static void test_media_frame_parses(void) {
  const char* json = "{\"v\":1,\"media\":{\"title\":\"Song\",\"artist\":\"Artist\",\"album\":\"Album\",\"playing\":true,\"positionSeconds\":42,\"durationSeconds\":240,\"lyrics\":\"line one\"}}";
  media_rec_t media;
  memset(&media, 0, sizeof(media));
  bool had = false;
  TEST_ASSERT_TRUE(hub_parse_media(json, strlen(json), &media, &had));
  TEST_ASSERT_TRUE(had);
  TEST_ASSERT_EQUAL_STRING("Song", media.title);
  TEST_ASSERT_EQUAL_STRING("Artist", media.artist);
  TEST_ASSERT_EQUAL_STRING("Album", media.album);
  TEST_ASSERT_TRUE(media.playing);
  TEST_ASSERT_EQUAL_UINT32(42, media.position_s);
  TEST_ASSERT_EQUAL_UINT32(240, media.duration_s);
  TEST_ASSERT_EQUAL_STRING("line one", media.lyrics);
}

static void test_media_fields_are_truncated(void) {
  const char* json = "{\"v\":1,\"media\":{\"title\":\"12345678901234567890123456789012345678901234567890123456789012345\",\"artist\":\"Artist\",\"album\":\"Album\",\"playing\":false}}";
  media_rec_t media;
  memset(&media, 0, sizeof(media));
  bool had = false;
  TEST_ASSERT_TRUE(hub_parse_media(json, strlen(json), &media, &had));
  TEST_ASSERT_TRUE(had);
  TEST_ASSERT_EQUAL(MEDIA_TITLE_LEN - 1, strlen(media.title));
  TEST_ASSERT_FALSE(media.playing);
}

static void test_media_frame_without_media_is_ignored(void) {
  const char* json = "{\"v\":1,\"sessions\":[]}";
  media_rec_t media;
  bool had = true;
  TEST_ASSERT_TRUE(hub_parse_media(json, strlen(json), &media, &had));
  TEST_ASSERT_FALSE(had);
}

static void test_media_frame_rejects_wrong_version(void) {
  const char* json = "{\"v\":2,\"media\":{\"title\":\"Song\"}}";
  media_rec_t media;
  bool had = false;
  TEST_ASSERT_FALSE(hub_parse_media(json, strlen(json), &media, &had));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_media_frame_parses);
  RUN_TEST(test_media_fields_are_truncated);
  RUN_TEST(test_media_frame_without_media_is_ignored);
  RUN_TEST(test_media_frame_rejects_wrong_version);
  return UNITY_END();
}
/* USER CODE END media_proto_tests */
