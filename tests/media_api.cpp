#include <boost/algorithm/string.hpp>

#include <chrono>
#include <fstream>
#include <streambuf>
#include <thread>

#include <gtest/gtest.h>

#include "client.hpp"
#include "mtx/requests.hpp"
#include "mtx/responses.hpp"

using namespace mtx::client;
using namespace mtx::identifiers;

using namespace std;

using ErrType = std::experimental::optional<errors::ClientError>;

string
get_media_id(const mtx::responses::ContentURI &res)
{
        vector<string> results;
        boost::split(results, res.content_uri, [](char c) { return c == '/'; });

        return results.back();
}

string
read_file(const string &file_path)
{
        ifstream file(file_path);
        string data((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());

        return data;
}

void
validate_upload(const mtx::responses::ContentURI &res, ErrType err)
{
        ASSERT_FALSE(err);
        ASSERT_TRUE(res.content_uri.size() > 10);
}

TEST(MediaAPI, UploadTextFile)
{
        std::shared_ptr<Client> alice = std::make_shared<Client>("localhost");

        alice->login("alice", "secret", [alice](const mtx::responses::Login &, ErrType err) {
                ASSERT_FALSE(err);

                const auto text = "This is some random text";

                alice->upload(text,
                              "text/plain",
                              "doc.txt",
                              [alice, text](const mtx::responses::ContentURI &res, ErrType err) {
                                      validate_upload(res, err);

                                      alice->download("localhost",
                                                      get_media_id(res),
                                                      [text](const string &data,
                                                             const string &content_type,
                                                             const string &original_filename,
                                                             ErrType err) {
                                                              ASSERT_FALSE(err);
                                                              EXPECT_EQ(data, text);
                                                              EXPECT_EQ(content_type, "text/plain");
                                                              EXPECT_EQ(original_filename,
                                                                        "doc.txt");
                                                      });
                              });
        });

        alice->close();
}

TEST(MediaAPI, UploadAudio)
{
        std::shared_ptr<Client> bob = std::make_shared<Client>("localhost");

        bob->login("bob", "secret", [bob](const mtx::responses::Login &, ErrType err) {
                ASSERT_FALSE(err);

                const auto audio = read_file("./fixtures/sound.mp3");

                bob->upload(audio,
                            "audio/mp3",
                            "sound.mp3",
                            [bob, audio](const mtx::responses::ContentURI &res, ErrType err) {
                                    validate_upload(res, err);

                                    bob->download("localhost",
                                                  get_media_id(res),
                                                  [audio](const string &data,
                                                          const string &content_type,
                                                          const string &original_filename,
                                                          ErrType err) {
                                                          ASSERT_FALSE(err);
                                                          EXPECT_EQ(data, audio);
                                                          EXPECT_EQ(content_type, "audio/mp3");
                                                          EXPECT_EQ(original_filename, "sound.mp3");
                                                  });
                            });
        });

        bob->close();
}

TEST(MediaAPI, UploadImage)
{
        std::shared_ptr<Client> carl = std::make_shared<Client>("localhost");

        carl->login("carl", "secret", [carl](const mtx::responses::Login &, ErrType err) {
                ASSERT_FALSE(err);

                const auto img = read_file("./fixtures/test.jpeg");

                carl->upload(img,
                             "image/jpeg",
                             "test.jpeg",
                             [carl, img](const mtx::responses::ContentURI &res, ErrType err) {
                                     validate_upload(res, err);

                                     carl->download("localhost",
                                                    get_media_id(res),
                                                    [img](const string &data,
                                                          const string &content_type,
                                                          const string &original_filename,
                                                          ErrType err) {
                                                            ASSERT_FALSE(err);
                                                            EXPECT_EQ(data, img);
                                                            EXPECT_EQ(content_type, "image/jpeg");
                                                            EXPECT_EQ(original_filename,
                                                                      "test.jpeg");
                                                    });
                             });
        });

        carl->close();
}
