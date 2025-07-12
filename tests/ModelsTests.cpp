#include "ModelsTests.h"

#include "models/ColorsModel/ColorsFilterModel.h"
#include "models/ColorsModel/ColorsModel.h"

void ModelsTests::TestColors() {
  const auto gen_colors_file = []() {
    ColorsModel::ColorsFile test_data;

    test_data.temperatures.push_back({ .name = "1500K", .temperature = 1500 });
    test_data.temperatures.push_back({ .name = "2700K", .temperature = 2700 });
    test_data.temperatures.push_back({ .name = "3400K", .temperature = 3400 });
    test_data.temperatures.push_back({ .name = "4500K", .temperature = 4500 });
    test_data.temperatures.push_back({ .name = "5600K", .temperature = 5600 });
    test_data.temperatures.push_back({ .name = "6500K", .temperature = 6500 });
    test_data.temperatures.push_back({ .name = "7500K", .temperature = 7500 });
    test_data.temperatures.push_back({ .name = "9000K", .temperature = 9000 });

    test_data.colors.push_back({ .name = "color1", .rgb = { .r = 1, .g = 2, .b = 3}});
    test_data.colors.push_back({ .name = "color2", .rgb = { .r = 121, .g = 2, .b = 3}});
    test_data.colors.push_back({ .name = "color3", .rgb = { .r = 4, .g = 210, .b = 234}});

    return test_data;
  };

  ColorsModel::ColorsFile colors_file = gen_colors_file();

  /* Test single temperature mode */

  // Note: We can meet situation like: min = 1200 = (2700 - 1500) / 2
  // Then, we can't definitely say which mode is the closest.
  // Docs doesn't give any notes on that, so can be a reason of a bug

  {
    ColorsModel source_model(colors_file);
    ColorsFilterModel filter_model;
    constexpr int single_value = 3321; // Closest number is 3400
    constexpr int target_value = 3400;

    filter_model.SetMin(single_value);
    filter_model.SetMax(single_value);
    filter_model.SetSupportsTemperature(true);
    filter_model.setSourceModel(&source_model);

    QCOMPARE(filter_model.rowCount(), 1);
    const QModelIndex index = filter_model.index(0, 0);
    QVariant color = filter_model.data(index, ColorsModel::TemperatureRole);
    QCOMPARE(color.toInt(), target_value);
  }

  {
    ColorsModel source_model(colors_file);
    ColorsFilterModel filter_model;
    constexpr int single_value = 4706; // Closest number is 4500
    constexpr int target_value = 4500;

    filter_model.SetMin(single_value);
    filter_model.SetMax(single_value);
    filter_model.SetSupportsTemperature(true);
    filter_model.setSourceModel(&source_model);

    QCOMPARE(filter_model.rowCount(), 1);
    const QModelIndex index = filter_model.index(0, 0);
    QVariant color = filter_model.data(index, ColorsModel::TemperatureRole);
    QCOMPARE(color.toInt(), target_value);
  }

  /* Test no temperatures/colors support */

  {
    ColorsModel source_model(colors_file);
    ColorsFilterModel filter_model;

    filter_model.SetSupportsTemperature(true);
    filter_model.SetSupportsColors(false);
    filter_model.setSourceModel(&source_model);

    QCOMPARE(filter_model.rowCount(), colors_file.temperatures.size());
  }

  {
    ColorsModel source_model(colors_file);
    ColorsFilterModel filter_model;

    filter_model.SetSupportsTemperature(false);
    filter_model.SetSupportsColors(true);
    filter_model.setSourceModel(&source_model);

    QCOMPARE(filter_model.rowCount(), colors_file.colors.size());
  }

  /* Just filtering */

  {
    ColorsModel source_model(colors_file);
    ColorsFilterModel filter_model;

    filter_model.SetMin(2000);
    filter_model.SetMax(7000);
    filter_model.SetSupportsTemperature(true);
    filter_model.SetSupportsColors(true);
    filter_model.setSourceModel(&source_model);

    // 5 - number of available temperatures for this case (min 200, max 7000)
    QCOMPARE(filter_model.rowCount(), 5 + colors_file.colors.size());
  }
}
