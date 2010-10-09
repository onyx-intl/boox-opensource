#ifndef DISTANCE_H
#define DISTANCE_H

typedef unsigned int gunichar;


class EditDistance
{
public:
    EditDistance( );
    ~EditDistance( );
    int CalEditDistance( const gunichar *s, const gunichar *t, const int limit );

private:
    int *d;
    int currentelements;
    /*Gets the minimum of three values */
    inline int minimum( const int a, const int b, const int c )
    {
        int min = a;
        if ( b < min )
            min = b;
        if ( c < min )
            min = c;
        return min;
    };

};

#endif
