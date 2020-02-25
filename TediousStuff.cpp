#include "TediousStuff.hpp"

#include <cassert>

namespace Tedious {

namespace {

inline int randomValue(int maxValue) { return rand() % maxValue; }

const std::vector<std::string> kStrings = {"foo", "bar", "baz",
                                           "some other string"};

bool operator==(const OO::Static::Model &staticModel,
                const std::unique_ptr<OO::Virtual::Model> &virtualModel) {
  if (std::holds_alternative<OO::Static::Text>(staticModel)) {
    auto sText = std::get<OO::Static::Text>(staticModel);
    auto vText = (OO::Virtual::Text *)virtualModel.get();

    return vText->text == sText.text && vText->x == sText.x &&
           vText->y == sText.y && vText->red == sText.red &&
           vText->green == sText.green && vText->blue == sText.blue &&
           vText->scale == sText.scale && vText->opacity == sText.opacity;
  } else {
    auto sImage = std::get<OO::Static::Image>(staticModel);
    auto vImage = (OO::Virtual::Image *)virtualModel.get();

    return vImage->imageSource == sImage.imageSource && vImage->x == sImage.x &&
           vImage->y == sImage.y && vImage->scale == sImage.scale &&
           vImage->opacity == sImage.opacity;
  }
}

bool operator==(const OO::Static::Model &staticModel,
                const OO::StaticWithSpan::Model &staticWithSpanModel) {
  if (std::holds_alternative<OO::Static::Text>(staticModel)) {
    auto sText = std::get<OO::Static::Text>(staticModel);
    auto swsText = std::get<OO::StaticWithSpan::Text>(staticWithSpanModel);

    return swsText.text == sText.text && swsText.x == sText.x &&
           swsText.y == sText.y && swsText.red == sText.red &&
           swsText.green == sText.green && swsText.blue == sText.blue &&
           swsText.scale == sText.scale && swsText.opacity == sText.opacity;
  } else {
    auto sImage = std::get<OO::Static::Image>(staticModel);
    auto swsImage = std::get<OO::StaticWithSpan::Image>(staticWithSpanModel);

    return swsImage.imageSource == sImage.imageSource &&
           swsImage.x == sImage.x && swsImage.y == sImage.y &&
           swsImage.scale == sImage.scale && swsImage.opacity == sImage.opacity;
  }
}

bool operator==(const OO::Static::Model &staticModel,
                const DataOriented::Model &dataOrientedModel) {
  if (std::holds_alternative<OO::Static::Text>(staticModel)) {
    auto sText = std::get<OO::Static::Text>(staticModel);
    auto dodText = std::get<DataOriented::Text>(dataOrientedModel);

    return dodText.text == dodText.text && dodText.properties[0] == sText.x &&
           dodText.properties[1] == sText.y &&
           dodText.properties[2] == sText.red &&
           dodText.properties[3] == sText.green &&
           dodText.properties[4] == sText.blue &&
           dodText.properties[5] == sText.scale &&
           dodText.properties[6] == sText.opacity;
  } else {
    auto sImage = std::get<OO::Static::Image>(staticModel);
    auto dodImage = std::get<DataOriented::Image>(dataOrientedModel);

    return dodImage.imageSource == sImage.imageSource &&
           dodImage.properties[0] == sImage.x &&
           dodImage.properties[1] == sImage.y &&
           dodImage.properties[2] == sImage.scale &&
           dodImage.properties[3] == sImage.opacity;
  }
}

auto makeInterpolator() {
  return LinearInterpolator{(float)(rand() % 256 - 256), (float)(rand() % 256)};
}

auto makeAnimations(const std::vector<Property> &properties) {
  auto animations = std::vector<OO::Animation>();
  animations.reserve(kNumberOfAnimationsPerModel);
  auto remainingAnimations = kNumberOfAnimationsPerModel;
  for (size_t j = 0; j < properties.size(); ++j) {
    auto property = properties[j];
    if (remainingAnimations == 0) {
      return animations;
    }
    auto currentAnimations = j == properties.size() - 1
                                 ? remainingAnimations
                                 : rand() % remainingAnimations;
    remainingAnimations -= currentAnimations;
    for (int i = 0; i < currentAnimations; ++i) {
      animations.push_back({makeInterpolator(), property});
    }
  }
  return animations;
}

template <typename StaticWithSpanModel>
void addAnimations(const std::vector<OO::Animation> &animations,
                   StaticWithSpanModel &model,
                   OO::StaticWithSpan::Input &input) {
  input.animations.insert(input.animations.end(), animations.cbegin(),
                          animations.cend());
  model.animations = gsl::span(
      input.animations.data() + input.animations.size() - animations.size(),
      animations.size());
}

template <typename DataOrientedModel>
void addAnimations(const std::vector<OO::Animation> &animations,
                   DataOrientedModel &model, DataOriented::Input &input) {
  auto numberOfPropertiesInModel = model.properties.size();
  size_t inputsBeginning = input.inputValues.size() - numberOfPropertiesInModel;
  int prevProperty = -1;
  size_t propCount = 0;
  for (const auto &animation : animations) {
    if (int propIndex = model.indexOfProperty(animation.propertyToAnimate);
        propIndex != prevProperty) {
      if (propCount > 0) {
        auto &inputValue = input.inputValues[inputsBeginning + prevProperty];
        inputValue.animations = gsl::span(
            input.interpolators.data() + inputsBeginning + prevProperty,
            propCount);
        assert(propCount == input.inputValues[inputsBeginning + prevProperty]
                                .animations.size());
      }
      prevProperty = propIndex;
      propCount = 0;
    }
    ++propCount;
    input.interpolators.push_back(animation.interpolator);
  }
  if (propCount > 0) {
    auto &inputValue = input.inputValues[inputsBeginning + prevProperty];
    inputValue.animations = gsl::span(
        input.interpolators.data() + inputsBeginning + prevProperty, propCount);
    assert(propCount ==
           input.inputValues[inputsBeginning + prevProperty].animations.size());
  }
}

}  // namespace

void makeModels(std::vector<std::unique_ptr<OO::Virtual::Model>> &virtualModels,
                std::vector<OO::Static::Model> &staticModels,
                OO::StaticWithSpan::Input &staticWithSpanInput,
                DataOriented::Input &dataOrientedInput) {
  virtualModels.reserve(kNumberOfModels);
  staticModels.reserve(kNumberOfModels);
  staticWithSpanInput.models.reserve(kNumberOfModels);
  staticWithSpanInput.animations.reserve(kTotalNumberOfAnimations);
  dataOrientedInput.models.reserve(kNumberOfModels);
  dataOrientedInput.inputValues = std::vector<DataOriented::InputValue>();
  dataOrientedInput.inputValues.reserve(kNumberOfModels * 7);
  dataOrientedInput.interpolators.reserve(kTotalNumberOfAnimations);
  dataOrientedInput.interpolationResults =
      std::vector<float>(kNumberOfModels * 7);

  int totalSetProperties = 0;
  for (int i = 0; i < kNumberOfModels; ++i) {
    int numberOfProperties = 0;
    if (rand() % 2 == 0) {
      printf("make text\n");
      numberOfProperties = 7;
      auto virtualModel = std::make_unique<OO::Virtual::Text>();
      virtualModel->text = kStrings[rand() % kStrings.size()];
      virtualModel->x = rand() % 100;
      virtualModel->y = rand() % 100;
      virtualModel->red = rand() % 256;
      virtualModel->green = rand() % 256;
      virtualModel->blue = rand() % 256;
      virtualModel->scale = rand() % 100;
      virtualModel->opacity = rand() % 100;

      auto staticModel = OO::Static::Text();
      staticModel.text = virtualModel->text;
      staticModel.x = virtualModel->x;
      staticModel.y = virtualModel->y;
      staticModel.red = virtualModel->red;
      staticModel.green = virtualModel->green;
      staticModel.blue = virtualModel->blue;
      staticModel.scale = virtualModel->scale;
      staticModel.opacity = virtualModel->opacity;

      auto staticWithSpanModel = OO::StaticWithSpan::Text();
      staticWithSpanModel.text = virtualModel->text;
      staticWithSpanModel.x = virtualModel->x;
      staticWithSpanModel.y = virtualModel->y;
      staticWithSpanModel.red = virtualModel->red;
      staticWithSpanModel.green = virtualModel->green;
      staticWithSpanModel.blue = virtualModel->blue;
      staticWithSpanModel.scale = virtualModel->scale;
      staticWithSpanModel.opacity = virtualModel->opacity;

      dataOrientedInput.inputValues.push_back({virtualModel->x,
                                               gsl::span<LinearInterpolator>(),
                                               UpdateOperator::Add});
      dataOrientedInput.inputValues.push_back({virtualModel->y,
                                               gsl::span<LinearInterpolator>(),
                                               UpdateOperator::Add});
      dataOrientedInput.inputValues.push_back({virtualModel->red,
                                               gsl::span<LinearInterpolator>(),
                                               UpdateOperator::Multiply});
      dataOrientedInput.inputValues.push_back({virtualModel->green,
                                               gsl::span<LinearInterpolator>(),
                                               UpdateOperator::Multiply});
      dataOrientedInput.inputValues.push_back({virtualModel->blue,
                                               gsl::span<LinearInterpolator>(),
                                               UpdateOperator::Multiply});
      dataOrientedInput.inputValues.push_back({virtualModel->scale,
                                               gsl::span<LinearInterpolator>(),
                                               UpdateOperator::Multiply});
      dataOrientedInput.inputValues.push_back({virtualModel->opacity,
                                               gsl::span<LinearInterpolator>(),
                                               UpdateOperator::Replace});
      auto dataOrientedModel = DataOriented::Text(
          virtualModel->text,
          gsl::span(dataOrientedInput.interpolationResults.data() +
                        totalSetProperties,
                    numberOfProperties));

      auto animations = makeAnimations({Property::X, Property::Y, Property::Red,
                                        Property::Green, Property::Blue,
                                        Property::Scale, Property::Opacity});
      virtualModel->animations = animations;
      staticModel.animations = animations;
      addAnimations(animations, staticWithSpanModel, staticWithSpanInput);
      addAnimations(animations, dataOrientedModel, dataOrientedInput);

      virtualModels.emplace_back(virtualModel.release());
      staticModels.emplace_back(staticModel);
      staticWithSpanInput.models.emplace_back(staticWithSpanModel);
      dataOrientedInput.models.push_back(dataOrientedModel);
    } else {
      numberOfProperties = 4;
      auto virtualModel = std::make_unique<OO::Virtual::Image>();
      virtualModel->imageSource = kStrings[rand() % kStrings.size()];
      virtualModel->x = rand() % 100;
      virtualModel->y = rand() % 100;
      virtualModel->scale = rand() % 100;
      virtualModel->opacity = rand() % 100;

      auto staticModel = OO::Static::Image();
      staticModel.imageSource = virtualModel->imageSource;
      staticModel.x = virtualModel->x;
      staticModel.y = virtualModel->y;
      staticModel.scale = virtualModel->scale;
      staticModel.opacity = virtualModel->opacity;

      auto staticWithSpanModel = OO::StaticWithSpan::Image();
      staticWithSpanModel.imageSource = virtualModel->imageSource;
      staticWithSpanModel.x = virtualModel->x;
      staticWithSpanModel.y = virtualModel->y;
      staticWithSpanModel.scale = virtualModel->scale;
      staticWithSpanModel.opacity = virtualModel->opacity;

      dataOrientedInput.inputValues.push_back({virtualModel->x,
                                               gsl::span<LinearInterpolator>(),
                                               UpdateOperator::Add});
      dataOrientedInput.inputValues.push_back({virtualModel->y,
                                               gsl::span<LinearInterpolator>(),
                                               UpdateOperator::Add});
      dataOrientedInput.inputValues.push_back({virtualModel->scale,
                                               gsl::span<LinearInterpolator>(),
                                               UpdateOperator::Replace});
      dataOrientedInput.inputValues.push_back({virtualModel->opacity,
                                               gsl::span<LinearInterpolator>(),
                                               UpdateOperator::Replace});

      auto dataOrientedModel = DataOriented::Image(
          virtualModel->imageSource,
          gsl::span(dataOrientedInput.interpolationResults.data() +
                        totalSetProperties,
                    numberOfProperties));

      auto animations = makeAnimations(
          {Property::X, Property::Y, Property::Scale, Property::Opacity});
      virtualModel->animations = animations;
      staticModel.animations = animations;
      addAnimations(animations, staticWithSpanModel, staticWithSpanInput);
      addAnimations(animations, dataOrientedModel, dataOrientedInput);

      virtualModels.emplace_back(virtualModel.release());
      staticModels.emplace_back(staticModel);
      staticWithSpanInput.models.emplace_back(staticWithSpanModel);
      dataOrientedInput.models.push_back(dataOrientedModel);
    }

    totalSetProperties += numberOfProperties;
  }
}

void verifyInitialModels(
    const std::vector<std::unique_ptr<OO::Virtual::Model>> &virtualModels,
    const std::vector<OO::Static::Model> &staticModels,
    const std::vector<OO::StaticWithSpan::Model> &staticWithSpanModels,
    const std::vector<DataOriented::InputValue> &dataOrientedInputs) {
  assert(virtualModels.size() == staticModels.size());
  assert(virtualModels.size() == staticWithSpanModels.size());

  size_t propertyCounter = 0;
  for (size_t i = 0; i < virtualModels.size(); ++i) {
    auto &staticModel = staticModels[i];
    assert(staticModel == staticWithSpanModels[i]);
    assert(staticModel == virtualModels[i]);

    if (std::holds_alternative<OO::Static::Text>(staticModel)) {
      auto sText = std::get<OO::Static::Text>(staticModel);

      assert(sText.x == dataOrientedInputs[propertyCounter++].initialValue);
      assert(sText.y == dataOrientedInputs[propertyCounter++].initialValue);
      assert(sText.red == dataOrientedInputs[propertyCounter++].initialValue);
      assert(sText.green == dataOrientedInputs[propertyCounter++].initialValue);
      assert(sText.blue == dataOrientedInputs[propertyCounter++].initialValue);
      assert(sText.scale == dataOrientedInputs[propertyCounter++].initialValue);
      assert(sText.opacity ==
             dataOrientedInputs[propertyCounter++].initialValue);
    } else {
      auto sImage = std::get<OO::Static::Image>(staticModel);

      assert(sImage.x == dataOrientedInputs[propertyCounter++].initialValue);
      assert(sImage.y == dataOrientedInputs[propertyCounter++].initialValue);
      assert(sImage.scale ==
             dataOrientedInputs[propertyCounter++].initialValue);
      assert(sImage.opacity ==
             dataOrientedInputs[propertyCounter++].initialValue);
    }
  }
}

void verifyModels(
    const std::vector<std::unique_ptr<OO::Virtual::Model>> &virtualModels,
    const std::vector<OO::Static::Model> &staticModels,
    const std::vector<OO::StaticWithSpan::Model> &staticWithSpanModels,
    const std::vector<DataOriented::Model> &dataOrientedModels) {
  assert(virtualModels.size() == staticModels.size());
  assert(virtualModels.size() == staticWithSpanModels.size());
  assert(virtualModels.size() == dataOrientedModels.size());

  size_t propertyCounter = 0;
  for (size_t i = 0; i < virtualModels.size(); ++i) {
    assert(staticModels[i] == staticWithSpanModels[i]);
    assert(staticModels[i] == virtualModels[i]);
    assert(staticModels[i] == dataOrientedModels[i]);
  }
}

}  // namespace Tedious
