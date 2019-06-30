#ifndef UHSM_UTILS_H
#define UHSM_UTILS_H

#include <type_traits>
#include <tuple>

namespace ushm::utils
{  
  template<typename MatchT, typename TupleT>
  struct contains {
    using type = std::false_type;
  };
  template<typename HeadT, typename... TailsTs>
  struct contains<HeadT, std::tuple<HeadT, TailsTs...>> {
    using type = std::true_type;
  };
  template<typename MatchT, typename HeadT, typename... TailTs>
  struct contains<MatchT, std::tuple<HeadT, TailTs...>> {
    using type = typename contains<MatchT, std::tuple<TailTs...>>::type;
  };
  template<typename MatchT, typename TupleT>
  using contains_t = typename contains<MatchT, TupleT>::type;
  
  template<typename AddedT, typename TupleT, typename = typename contains<AddedT, TupleT>::type>
  struct add_unique;
  template<typename AddedT, typename... TupleTs>
  struct add_unique<AddedT, std::tuple<TupleTs...>, std::true_type> {
    using type = std::tuple<TupleTs...>;
  };
  template<typename AddedT, typename... TupleTs>
  struct add_unique<AddedT, std::tuple<TupleTs...>, std::false_type> {
    using type = std::tuple<AddedT, TupleTs...>;
  };
  template<typename AddedT, typename TupleT>
  using add_unique_t = typename add_unique<AddedT, TupleT>::type;
  
//  template<typename TupleT, typename UniqueT>
//  struct remove_duplicates;
//  
//  template<typename HeadT, typename... TailTs, typename... UniqueTs>
//  struct remove_duplicates<std::tuple<HeadT, TailTs...>, add_unique_t<HeadT, std::tuple<TailTs...>>>  {
//    
//  };
  
  template<typename TupleT>
  struct remove_duplicates;
  template<typename HeadT, typename... TailTs>
  struct remove_duplicates<std::tuple<HeadT, TailTs...>>  {
    using type = add_unique_t<HeadT, typename remove_duplicates<std::tuple<TailTs...>>::type>;
  };
  template<typename TupleT>
  using remove_duplicates_t = typename remove_duplicates<TupleT>::type;
  
  
  // TESTS
  
  namespace Common
  {
    using Basic_tuple = std::tuple<bool, int, float>;
    using Tuple_w_duplicates = std::tuple<char, int, int, bool>;
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
}

#endif
