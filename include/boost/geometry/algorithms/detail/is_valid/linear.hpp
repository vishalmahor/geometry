// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_IS_VALID_LINEAR_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_IS_VALID_LINEAR_HPP

#include <cstddef>
#include <algorithm>

#include <boost/assert.hpp>
#include <boost/range.hpp>

#include <boost/geometry/core/closure.hpp>
#include <boost/geometry/core/point_type.hpp>
#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/util/range.hpp>

#include <boost/geometry/algorithms/equals.hpp>
#include <boost/geometry/algorithms/detail/check_iterator_range.hpp>
#include <boost/geometry/algorithms/detail/is_valid/has_spikes.hpp>

#include <boost/geometry/algorithms/dispatch/is_valid.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace is_valid
{


// returns the number of distinct points in the range;
// return values are zero, one, two, three_or_more
template <typename Range>
struct number_of_distinct_points
{
    static inline std::size_t apply(Range const& range)
    {
        typedef typename point_type<Range>::type point;
        typedef typename boost::range_iterator<Range const>::type iterator;

        std::size_t size = boost::size(range);

        if ( size < 2u )
        {
            return size;
        }

        iterator it1 =
            std::find_if(boost::begin(range),
                         boost::end(range),
                         not_equal_to<point>(range::front(range)));

        if ( it1 == boost::end(range) )
        {
            return 1u;
        }

        iterator it2 = 
            std::find_if(it1, boost::end(range), not_equal_to<point>(*it1));

        return
            ( it2 == boost::end(range)
              || geometry::equals(range::front(range), *it2) )
            ?
            2u
            :
            3u
            ;
    }
};


template <typename Linestring, bool AllowSpikes>
struct is_valid_linestring
{
    static inline bool apply(Linestring const& linestring)
    {
        typedef number_of_distinct_points<Linestring> num_distinct;

        std::size_t linestring_size = num_distinct::apply(linestring);

        if ( linestring_size < 2u )
        {
            return false;
        }

        if ( !AllowSpikes && linestring_size == 2u )
        {
            return !geometry::equals(range::front(linestring),
                                     range::back(linestring));
        }

        return AllowSpikes
            || !has_spikes<Linestring, closed>::apply(linestring);
    }
};


}} // namespace detail::is_valid
#endif // DOXYGEN_NO_DETAIL




#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


// A linestring is a curve.
// A curve is 1-dimensional so it has to have at least two distinct
// points.
// A curve is simple if it does not pass through the same point twice,
// with the possible exception of its two endpoints
//
// There is an option here as to whether spikes are allowed for linestrings; 
// here we pass this as an additional template parameter: allow_spikes
// If allow_spikes is set to true, spikes are allowed, false otherwise.
// By default, spikes are disallowed
//
// Reference: OGC 06-103r4 (§6.1.6.1)
template <typename Linestring, bool AllowSpikes>
struct is_valid<Linestring, linestring_tag, AllowSpikes>
    : detail::is_valid::is_valid_linestring<Linestring, AllowSpikes>
{};


// A MultiLinestring is a MultiCurve
// A MultiCurve is simple if all of its elements are simple and the
// only intersections between any two elements occur at Points that
// are on the boundaries of both elements.
//
// Reference: OGC 06-103r4 (§6.1.8.1; Fig. 9)
template <typename MultiLinestring, bool AllowSpikes>
struct is_valid<MultiLinestring, multi_linestring_tag, AllowSpikes>
{
    static inline bool apply(MultiLinestring const& multilinestring)
    {
        return detail::check_iterator_range
            <
                detail::is_valid::is_valid_linestring
                    <
                        typename boost::range_value<MultiLinestring>::type,
                        AllowSpikes
                    >
            >::apply(boost::begin(multilinestring),
                     boost::end(multilinestring));
    }
};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_IS_VALID_LINEAR_HPP
