#ifndef RatelOptionalH
#define RatelOptionalH
//-------------------------------------------------------------------
#include <stdexcept>
#include <utility>
//-------------------------------------------------------------------
struct optional_exception : public std::logic_error
{
  optional_exception() : logic_error("Bad optional access") { }

  explicit optional_exception(const char* arg) : logic_error(arg) { }

  virtual ~optional_exception() noexcept = default;
};
//-------------------------------------------------------------------
struct InPlaceT {};

constexpr InPlaceT inPlace;
//-------------------------------------------------------------------
struct NullOptT
{
  enum class guard_ { token_ };

  explicit constexpr NullOptT(guard_) {}
};

constexpr NullOptT nullOpt(NullOptT::guard_::token_);
//-------------------------------------------------------------------
namespace detail
{
//-------------------------------------------------------------------
template<typename ...> struct and__;

template<> struct and__<> : public std::true_type {};

template<typename T1__> struct and__<T1__> : public T1__ {};

template<typename T1__, typename T2__>
struct and__<T1__, T2__> : public std::conditional<T1__::value, T2__, T1__>::type {};

template<typename T1__, typename T2__, typename T3__, typename ... Tn__>
struct and__<T1__, T2__, T3__, Tn__...> :
  public std::conditional<T1__::value, and__<T2__, T3__, Tn__...>, T1__>::type
{};

template<typename T__>
struct not__ : public std::integral_constant<bool, !T__::value> {};
//-------------------------------------------------------------------
template<typename T__> struct Optional;

template<typename T__> struct isOptionalImpl : std::false_type {};

template<typename T__> struct isOptionalImpl<Optional<T__>> : std::true_type {};

template<typename T__> struct isOptional :
    public isOptionalImpl<typename std::remove_cv<typename std::remove_reference<T__>::type>::type>
{};
//-------------------------------------------------------------------
template<typename T__>
class OptionalBase__
{
  using StoredT__ = typename std::remove_const<T__>::type;
public:

  constexpr OptionalBase__() noexcept :
    mEmpty_{}
  {}

  constexpr OptionalBase__(NullOptT) noexcept :
    OptionalBase__{}
  {}

  constexpr OptionalBase__(const T__ & t_) :
    mStored_(t_),
    mEngaged_(true)
  {}

  constexpr OptionalBase__(T__ && t_) :
    mStored_(std::move(t_)),
    mEngaged_(true)
  {}

  template<typename ... Args__>
  constexpr explicit OptionalBase__(InPlaceT, Args__ &&... args_) :
    mStored_(std::forward<Args__>(args_)...),
    mEngaged_(true)
  {}

  OptionalBase__(const OptionalBase__ & other_)
  {
    if (other_.mEngaged_)
      this->construct__(other_.get__());
  }

  OptionalBase__(OptionalBase__ && other_)
  noexcept(std::is_nothrow_move_constructible<T__>())
  {
    if (other_.mEngaged_)
      this->construct__(std::move(other_.get__()));
  }

  OptionalBase__ & operator =(const OptionalBase__ & other_)
  {
    if (this->mEngaged_ && other_.mEngaged_)
      this->get__() = other_.get__();
    else
    {
      if (other_.mEngaged_)
        this->construct__(other_.get__());
      else
        this->reset__();
    }

    return *this;
  }

  OptionalBase__ & operator =(OptionalBase__ && other_)
  noexcept(std::is_nothrow_move_constructible<T__>() && std::is_nothrow_move_assignable<T__>())
  {
    if (this->mEngaged_ && other_.mEngaged_)
      this->get__() = std::move(other_.get__());
    else
    {
      if (other_.mEngaged_)
        this->construct__(std::move(other_.get__()));
      else
        this->reset__();
    }

    return *this;
  }

  ~OptionalBase__()
  {
    if (this->mEngaged_)
      this->mStored_.~StoredT__();
  }

protected:
  constexpr bool isEngaged__() const noexcept
  {
    return this->mEngaged_;
  }

  T__ & get__() noexcept
  {
    return mStored_;
  }

  const T__ & get__() const noexcept
  {
    return mStored_;
  }

  template<typename ... Args__>
  void construct__(Args__ &&... args_)
  noexcept(std::is_nothrow_constructible<StoredT__, Args__...>())
  {
    ::new (std::addressof(this->mStored_)) StoredT__(std::forward<Args__>(args_)...);
    this->mEngaged_ = true;
  }

  void destruct__()
  {
    this->mEngaged_ = false;
    this->mStored_.~StoredT__();
  }

  void reset__()
  {
    if (this->mEngaged_)
      this->destruct__();
  }

private:
  struct EmptyByte__ {};

  union
  {
    EmptyByte__ mEmpty_;
    StoredT__ mStored_;
  };

  bool mEngaged_ = false;
};
//-------------------------------------------------------------------
}
//-------------------------------------------------------------------
template<typename T__>
class Optional : private detail::OptionalBase__<T__>
{
  static_assert(
    detail::and__<
      detail::not__<std::is_same<typename std::remove_cv<T__>::type, NullOptT>>,
      detail::not__<std::is_same<typename std::remove_cv<T__>::type, InPlaceT>>,
      detail::not__<std::is_reference<T__>>
    >(),
    "Invalid type for optional"
  );

  using Base__ = detail::OptionalBase__<T__>;
public:
  using Base__::Base__;

  constexpr Optional() = default;

  template<typename U__,
           typename std::enable_if<detail::and__<
             detail::not__<std::is_same<T__, U__>>,
             std::is_constructible<T__, U__&&>,
             std::is_convertible<U__&&, T__>
           >::value, bool>::type = true>
  constexpr Optional(U__ && t_) :
    Base__(T__(std::forward<U__>(t_)))
  {}

  template<typename U__,
           typename std::enable_if<detail::and__<
             detail::not__<std::is_same<T__, U__>>,
             std::is_constructible<T__, U__&&>,
             detail::not__<std::is_convertible<U__&&, T__>>
           >::value, bool>::type = false>
  explicit constexpr Optional(U__ && t_) :
    Base__(T__(std::forward<U__>(t_)))
  {}

  template <typename U__,
            typename std::enable_if<detail::and__<
              detail::not__<std::is_same<T__, U__>>,
              detail::not__<std::is_constructible<
                T__, const Optional<U__>&>>,
              detail::not__<std::is_convertible<
                const Optional<U__>&, T__>>,
              std::is_constructible<T__, const U__&>,
              std::is_convertible<const U__&, T__>
            >::value, bool>::type = true>
  constexpr Optional(const Optional<U__> & t_) :
    Base__(t_ ? Optional<T__>(*t_) : Optional<T__>())
  {}

  template <typename U__,
            typename std::enable_if<detail::and__<
              detail::not__<std::is_same<T__, U__>>,
              detail::not__<std::is_constructible<
                 T__, Optional<U__>&&>>,
              detail::not__<std::is_convertible<
                 Optional<U__>&&, T__>>,
              std::is_constructible<T__, U__&&>,
              std::is_convertible<U__&&, T__>
            >::value, bool>::type = true>
  constexpr Optional(Optional<U__> && t_) :
    Base__(t_ ? Optional<T__>(std::move(*t_)) : Optional<T__>())
  {}

  template <typename U__,
            typename std::enable_if<detail::and__<
              detail::not__<std::is_same<T__, U__>>,
              detail::not__<std::is_constructible<
                 T__, Optional<U__>&&>>,
              detail::not__<std::is_convertible<
                 Optional<U__>&&, T__>>,
              std::is_constructible<T__, U__&&>,
              detail::not__<std::is_convertible<U__&&, T__>>
            >::value, bool>::type = false>
  explicit constexpr Optional(Optional<U__> && t_) :
    Base__(t_ ? Optional<T__>(std::move(*t_)) : Optional<T__>())
  {}

  Optional & operator =(NullOptT) noexcept
  {
    this->reset__();
    return *this;
  }

  template<typename U__,
           typename std::enable_if<detail::and__<
             detail::not__<std::is_same<U__, NullOptT>>,
             detail::not__<detail::isOptional<U__>>
           >::value,
           bool>::type = true>
  Optional & operator =(U__ && u_)
  {
    static_assert(
      detail::and__<
        std::is_constructible<T__, U__>,
        std::is_assignable<T__&, U__>
      >(),
      "Can't assign to value type from argument"
    );
    return *this;
  }

  template<typename U__,
           typename std::enable_if<detail::and__<
             detail::not__<std::is_same<U__, T__>>>::value,
           bool>::type = true>
  Optional & operator =(const Optional<U__> & t_)
  {
    static_assert(
      detail::and__<
        std::is_constructible<T__, U__>,
        std::is_assignable<T__&, U__>
      >(),
      "Can't assign to value type from argument"
    );

    if (this->isEngaged__())
      this->get__() = std::forward<U__>(t_);
    else
      this->construct__(std::forward<U__>(t_));

    return *this;
  }

  template<typename U__,
           typename std::enable_if<detail::and__<
             detail::not__<std::is_same<U__, T__>>>::value,
           bool>::type = true>
  Optional & operator =(Optional<U__> && t_)
  {
    static_assert(
      detail::and__<
        std::is_constructible<T__, U__>,
        std::is_assignable<T__&, U__>
      >(),
      "Can't assign to value type from argument"
    );

    if (t_)
    {
      if (this->isEngaged__())
        this->get__() = std::move(*t_);
      else
        this->construct__(std::move(*t_));
    }
    else
    {
      this->reset__();
    }

    return *this;
  }

  template<typename ... Args__>
  void emplace(Args__ &&... args_)
  {
    static_assert(
      std::is_constructible<T__, Args__&&...>(),
      "Can't emplace value type from arguments"
    );

    this->reset__();
    this->construct__(std::forward<Args__>(args_)...);

  }

  void swap(Optional & other_)
  noexcept(std::is_nothrow_move_constructible<T__>() && noexcept(std::swap(std::declval<T__&>(), std::declval<T__&>())))
  {
    using std::swap;

    if (this->isEngaged__() && other_.isEngaged__())
      swap(this->get__(), other_.get__());
    else if (this->isEngaged__())
    {
      other_.construct__(std::move(this->get__()));
      this->destruct__();
    }
    else if (other_.isEngaged__())
    {
      this->construct__(std::move(other_.get__()));
      other_.destruct__();
    }
  }

  const T__ * operator ->() const
  {
    return std::addressof(this->get__());
  }

  T__ * operator ->()
  {
    return std::addressof(this->get__());
  }

  const T__ & operator *() const
  {
    return this->get__();
  }

  T__ & operator *()
  {
    return this->get__();
  }

  constexpr explicit operator bool() const noexcept
  {
    return this->isEngaged__();
  }

  const T__ & value() const
  {
    if (!*this)
      throw optional_exception("Attempt to access value of a disengaged optional object");

    return **this;
  }

  T__ & value()
  {
    if (!*this)
      throw optional_exception("Attempt to access value of a disengaged optional object");

    return **this;
  }

  template<typename U__>
  constexpr T__ value_or(U__ && t_) const &
  {
    static_assert(
      detail::and__<
        std::is_copy_constructible<T__>,
        std::is_convertible<U__&&, T__>
      >(),
      "Cannot return value"
    );

    return this->isEngaged__() ?
             this->get__() :
             static_cast<T__>(std::forward<U__>(t_));
  }

  template<typename U__>
  constexpr T__ value_or(U__ && t_) &&
  {
    static_assert(
      detail::and__<
        std::is_move_constructible<T__>,
        std::is_convertible<U__&&, T__>
      >(),
      "Cannot return value"
    );

    return this->isEngaged__() ?
             std::move(this->get__()) :
             static_cast<T__>(std::forward<U__>(t_));
  }
};
//-------------------------------------------------------------------
template<typename T__>
constexpr bool operator ==(const Optional<T__> & l_, const Optional<T__> & r_)
{
  return static_cast<bool>(l_) == static_cast<bool>(r_) && (!l_ || *l_ == *r_);
}

template<typename T__>
constexpr bool operator !=(const Optional<T__> & l_, const Optional<T__> & r_)
{
  return !(l_ == r_);
}

template<typename T__>
constexpr bool operator <(const Optional<T__> & l_, const Optional<T__> & r_)
{
  return static_cast<bool>(r_) && (!l_ || *l_ < *r_);
}

template<typename T__>
constexpr bool operator >(const Optional<T__> & l_, const Optional<T__> & r_)
{
  return (r_ < l_);
}

template<typename T__>
constexpr bool operator <=(const Optional<T__> & l_, const Optional<T__> & r_)
{
  return !(r_ < l_);
}

template<typename T__>
constexpr bool operator >=(const Optional<T__> & l_, const Optional<T__> & r_)
{
  return !(l_ < r_);
}

template<typename T__>
constexpr bool operator ==(const Optional<T__> & l_, const T__ & r_)
{
  return l_ && *l_ == r_;
}

template<typename T__>
constexpr bool operator ==(const T__ & l_, const Optional<T__> & r_)
{
  return r_ && l_ == *r_;
}

template<typename T__>
constexpr bool operator !=(const Optional<T__> & l_, const T__ & r_)
{
  return !(r_ == l_);
}

template<typename T__>
constexpr bool operator !=(const T__ & l_, const Optional<T__> & r_)
{
  return !r_ || !(l_ == *r_);
}

template<typename T__>
constexpr bool operator <(const Optional<T__> & l_, const T__ & r_)
{
  return l_ && *l_ < r_;
}

template<typename T__>
constexpr bool operator <(const T__ & l_, const Optional<T__> & r_)
{
  return r_ && l_ < *r_;
}

template<typename T__>
constexpr bool operator >(const Optional<T__> & l_, const T__ & r_)
{
  return l_ && r_ < *l_;
}

template<typename T__>
constexpr bool operator >(const T__ & l_, const Optional<T__> & r_)
{
  return !r_ || *r_ < l_;
}

template<typename T__>
constexpr bool operator <=(const Optional<T__> & l_, const T__ & r_)
{
  return l_ && !(l_ < r_);
}

template<typename T__>
constexpr bool operator <=(const T__ & l_, const Optional<T__> & r_)
{
  return r_ && !(*r_ < l_);
}

template<typename T__>
constexpr bool operator >=(const Optional<T__> & l_, const T__ & r_)
{
  return l_ && !(*l_ < r_);
}

template<typename T__>
constexpr bool operator >=(const T__ & l_, const Optional<T__> & r_)
{
  return !r_ || !(l_ < *r_);
}

template<typename T__>
constexpr bool operator ==(const Optional<T__> & l_, NullOptT)
{
  return !l_;
}

template<typename T__>
constexpr bool operator ==(NullOptT, const Optional<T__> & r_)
{
  return !r_;
}

template<typename T__>
constexpr bool operator !=(const Optional<T__> & l_, NullOptT)
{
  return static_cast<bool>(l_);
}

template<typename T__>
constexpr bool operator !=(NullOptT, const Optional<T__> & r_)
{
  return static_cast<bool>(r_);
}

template<typename T__>
constexpr bool operator <(const Optional<T__> &, NullOptT)
{
  return false;
}

template<typename T__>
constexpr bool operator <(NullOptT, const Optional<T__> & r_)
{
  return static_cast<bool>(r_);
}

template<typename T__>
constexpr bool operator >(const Optional<T__> & l_, NullOptT)
{
  return static_cast<bool>(l_);
}

template<typename T__>
constexpr bool operator >(NullOptT, const Optional<T__> &)
{
  return false;
}

template<typename T__>
constexpr bool operator <=(const Optional<T__> & l_, NullOptT)
{
  return !l_;
}

template<typename T__>
constexpr bool operator <=(NullOptT, const Optional<T__> &)
{
  return true;
}

template<typename T__>
constexpr bool operator >=(const Optional<T__> &, NullOptT)
{
  return true;
}

template<typename T__>
constexpr bool operator >=(NullOptT, const Optional<T__> & r_)
{
  return !r_;
}
//-------------------------------------------------------------------
template<typename T__, typename ... Args__>
Optional<T__> make_optional(Args__ && ... args_)
{
  return Optional<typename std::decay<T__>::type>(std::forward<Args__>(args_)...);
}
//-------------------------------------------------------------------
#endif // RatelOptionalH
