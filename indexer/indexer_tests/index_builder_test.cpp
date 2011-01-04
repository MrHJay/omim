#include "../../testing/testing.hpp"

#include "../index.hpp"
#include "../index_builder.hpp"
#include "../classif_routine.hpp"
#include "../features_vector.hpp"
#include "../data_header_reader.hpp"
#include "../../storage/defines.hpp"
#include "../../platform/platform.hpp"
#include "../../coding/file_container.hpp"
#include "../../base/stl_add.hpp"


UNIT_TEST(BuildIndexTest)
{
  Platform & p = GetPlatform();
  classificator::Read(p.ReadPathForFile("drawing_rules.bin"),
                      p.ReadPathForFile("classificator.txt"),
                      p.ReadPathForFile("visibility.txt"));

  FilesContainerR originalContainer(p.WritablePathForFile("minsk-pass" DATA_FILE_EXTENSION));

  // Build index.
  vector<char> serialIndex;
  {
    FeaturesVector<FileReader> featuresVector(originalContainer);
    MemWriter<vector<char> > serialWriter(serialIndex);
    indexer::BuildIndex(featuresVector, serialWriter, "build_index_test");
  }

  // Create a new mwm file.
  string const fileName = p.WritablePathForFile("build_index_test" DATA_FILE_EXTENSION);
  FileWriter::DeleteFile(fileName);

  // Copy original mwm file and replace index in it.
  {
    FilesContainerW containerWriter(fileName);
    vector<string> tags;
    originalContainer.ForEachTag(MakeBackInsertFunctor(tags));
    for (size_t i = 0; i < tags.size(); ++i)
    {
      if (tags[i] != INDEX_FILE_TAG)
      {
        FileReader reader = originalContainer.GetReader(tags[i]);
        vector<char> data(reader.Size());
        reader.Read(0, &data[0], data.size());
        containerWriter.Append(data, tags[i]);
      }
    }
    containerWriter.Append(serialIndex, INDEX_FILE_TAG);
  }

  // Check that index actually works.
  Index<FileReader>::Type index;
  index.Add(fileName);

  // Make sure that index is actually parsed.
  index.ForEachInScale(NoopFunctor(), 15);

  // Clean after the test.
  FileWriter::DeleteFile(fileName);
}
