#ifndef UHSM_UTILS_H
#define UHSM_UTILS_H

#include <type_traits>
#include <tuple>

namespace ushm::utils
{ 
  // NOTE: checks if a std::tuple contains a type; assumes that it does not
  // unless 'proven' otherwise
  template<typename MatchT, typename TupleT>
  struct contains {
    using type = std::false_type;
  };
  // matched type at tuple's head, true
  template<typename HeadT, typename... TailsTs>
  struct contains<HeadT, std::tuple<HeadT, TailsTs...>> {
    using type = std::true_type;
  };
  // not matched at tuple's head, check the tail
  template<typename MatchT, typename HeadT, typename... TailTs>
  struct contains<MatchT, std::tuple<HeadT, TailTs...>> {
    using type = typename contains<MatchT, std::tuple<TailTs...>>::type;
  };
  
  template<typename MatchT, typename TupleT>
  using contains_t = typename contains<MatchT, TupleT>::type;
  
  // NOTE: uniquely adds a type at the original tuple's head
  template<typename AddedT, typename TupleT, typename = typename contains<AddedT, TupleT>::type>
  struct add_unique;
  // original tuple does contain the type being added, leave original tuple unchanged
  template<typename AddedT, typename... TupleTs>
  struct add_unique<AddedT, std::tuple<TupleTs...>, std::true_type> {
    using type = std::tuple<TupleTs...>;
  };
  // original tuple does not contain the type being added, add it at the tuple's head
  template<typename AddedT, typename... TupleTs>
  struct add_unique<AddedT, std::tuple<TupleTs...>, std::false_type> {
    using type = std::tuple<AddedT, TupleTs...>;
  };
  
  template<typename AddedT, typename TupleT>
  using add_unique_t = typename add_unique<AddedT, TupleT>::type;
  
  // NOTE: removes duplicates from an std::tuple  
  template<typename TupleT>
  struct remove_duplicates;
  // uniquely add current type to the tuple's tail with already removed duplicates
  template<typename HeadT, typename... TailTs>
  struct remove_duplicates<std::tuple<HeadT, TailTs...>>  {
    using type = add_unique_t<HeadT, typename remove_duplicates<std::tuple<TailTs...>>::type>;
  };
  // a tuple with a single type cannot have duplicates
  template<typename TailT>
  struct remove_duplicates<std::tuple<TailT>> {
    using type = std::tuple<TailT>;
  };
  
  template<typename TupleT>
  using remove_duplicates_t = typename remove_duplicates<TupleT>::type;
  
  
  // Tests
  ////////////////////////////////////////////////////////////////////////////////
  
  namespace Common
  {
    using Basic_tuple = std::tuple<bool, int, float>;
    // NOTE: duplicate types occur both adjacently and non-adjacently on the parameter list
    using Tuple_w_duplicates = std::tuple<int, char, float, int, int, bool, float, float>;
  }
  
  namespace TestContains_DoContain_ReturnTrue
  {
    using Bool_contains = contains_t<int, Common::Basic_tuple>;
    static_assert(std::is_same_v<Bool_contains, std::true_type>);
  }
  
  namespace TestContains_DoNotContain_ReturnFalse
  {
    using Bool_contains = contains_t<char, Common::Basic_tuple>;
    static_assert(std::is_same_v<Bool_contains, std::false_type>);
  }
  
  namespace TestAddUnique_AddAlreadyPresent_TupleUnchanged
  {
    using New_tuple = add_unique_t<float, Common::Basic_tuple>;
    static_assert(std::is_same_v<New_tuple, Common::Basic_tuple>);
  }
  
  namespace TestAddUnique_AddNotPresent_AddedAtHead
  {
    using New_tuple = add_unique_t<char, Common::Basic_tuple>;
    static_assert(std::is_same_v<New_tuple, std::tuple<char, bool, int, float>>);
  }
  
  namespace TestRemoveDuplicates_NoDuplicates_TupleUnchanged
  {
    using New_tuple = remove_duplicates_t<Common::Basic_tuple>;
    static_assert(std::is_same_v<New_tuple, Common::Basic_tuple>);
  }
  
  namespace TestRemoveDuplicates_DuplicatesPresent_DuplicatesRemoved
  {
    using New_tuple = remove_duplicates_t<Common::Tuple_w_duplicates>;
    static_assert(std::is_same_v<New_tuple, std::tuple<char, int, bool, float>>);
  }
}

#endif
