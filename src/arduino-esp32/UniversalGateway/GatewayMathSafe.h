/**
   USE OF THIS SOFTWARE IS GOVERNED BY THE TERMS AND CONDITIONS
   OF THE LICENSE STATEMENT AND LIMITED WARRANTY FURNISHED WITH
   THE PRODUCT.
   <p/>
   IN PARTICULAR, YOU WILL INDEMNIFY AND HOLD B2N LTD., ITS
   RELATED COMPANIES AND ITS SUPPLIERS, HARMLESS FROM AND AGAINST ANY
   CLAIMS OR LIABILITIES ARISING OUT OF THE USE, REPRODUCTION, OR
   DISTRIBUTION OF YOUR PROGRAMS, INCLUDING ANY CLAIMS OR LIABILITIES
   ARISING OUT OF OR RESULTING FROM THE USE, MODIFICATION, OR
   DISTRIBUTION OF PROGRAMS OR FILES CREATED FROM, BASED ON, AND/OR
   DERIVED FROM THIS SOURCE CODE FILE.
*/

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Safe implementation of math fuctions without macros


// abs not allowed for unsigned
#define abs_safe_def( t ) \
  inline t abs_safe(t v) { return v < 0 ? -v : v; }

abs_safe_def( int8_t )
abs_safe_def( int16_t )
abs_safe_def( int32_t )
abs_safe_def( int64_t )
abs_safe_def( float )
abs_safe_def( double )

// round 
#define round_safe_def( t ) inline t round_safe(t v) { return (t) ((v)>=0?(long)((v)+0.5):  (long)((v)-0.5)); }

round_safe_def( float )
round_safe_def( double )


  
