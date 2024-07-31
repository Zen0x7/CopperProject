#pragma once

#include <string>

namespace elephant {

  /**  Language codes to be used with the Elephant class */
  enum class LanguageCode { EN, DE, ES, FR };

  /**
   * @brief A class for saying hello in multiple languages
   */
  class Elephant {
    std::string name;

  public:
    /**
     * @brief Creates a new elephant
     * @param name the name to greet
     */
    Elephant(std::string name);

    /**
     * @brief Creates a localized string containing the greeting
     * @param lang the language to greet in
     * @return a string containing the greeting
     */
    std::string greet(LanguageCode lang = LanguageCode::EN) const;
  };

}  // namespace elephant
