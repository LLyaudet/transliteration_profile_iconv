<?php

try{

  //check input
  if(!isset($argv[1])){
    throw new Exception('Please give the input file name (arg 1).');
  }
  $sInputFileName = $argv[1];

  if(!isset($argv[2])){
    throw new Exception('Please give the output file name (arg 2).');
  }
  $sOutputFileName = $argv[2];

  if(!isset($argv[3])){
    throw new Exception('Please give the unicode encoding (arg 3).');
  }
  $sUnicodeEncoding = $argv[3];
  $arrSValidEncoding = [ 
    'UTF-8',
    'UTF-16BE',
    'UTF-16LE',
    'UTF-32BE',
    'UTF-32LE',
  ];
  if(!in_array($sUnicodeEncoding, $arrSValidEncoding)){
    throw new Exception('This encoding is not supported.');
  }

  if(!isset($argv[4])){
    throw new Exception('Please give the default value (arg 4).');
  }
  $sDefaultValue = $argv[4];
  
  if(ctype_xdigit($sDefaultValue)){
    $iLength = strlen($sDefaultValue);
    if($iLength % 2 === 1){
      throw new Exception('The default value must have a *pair* number of hexadecimal digits.');
    }
    if($iLength > 254){//127 octets max
      throw new Exception('The default value must not represent more than 127 octets (254 hexadecimal digits).');
    }
  }
  else{
    $sSigne = substr($sDefaultValue, 0, 1);
    $sEntier = substr($sDefaultValue, 1);
    if($sSigne !== '-' || !ctype_digit($sEntier)){
      throw new Exception('The default value must be hexadecimal or a negative integer.');
    }
    if(strlen($sEntier) > 3 || ((int)$sEntier) > 128){
      throw new Exception('The default value must be greater than -128.');
    }
  }


  //do the job
  $sInputContent = file_get_contents($sInputFileName);
  if($sInputContent == ''){
    throw new Exception('The input file is empty.');
  }

  $sOutputContent = '';


  $arrSLines = explode("\n", $sInputContent);
  $i = 0;
  foreach($arrSLines as $sLine){
    if($sLine === ''){
      continue;
    }
    ++$i;
    try{
      $sOutputContent .= 
          getSHexadecimalStringFrom($sLine, $sUnicodeEncoding)
         .' '.$sDefaultValue."\n";
      ;
    }
    catch(Exception $oException){
      throw new Exception(
          sprintf('There was an error while reading line %s.', $i)
         .' '.$oException->getMessage()
      );
    }
  }


  if(!file_put_contents($sOutputFileName, $sOutputContent)){
    throw new Exception('There was an error when trying to write the output file.');
  }

}
catch(Exception $oException){
  echo $oException->getMessage(), "\n";

  echo "------------------------------------------------------------\n",
       "----- Script fromUnicodeDataToDefaultProfile           -----\n",
       "------------------------------------------------------------\n",
       "Usage : php fromUnicodeDataToDefaultProfile.php @InputFileName@ @OutputFileName@ @UnicodeEncoding@ @DefaultValue@\n",
       "@InputFileName@ : name of the input file with unicode data, e.g. \"UnicodeData_10.0.0.txt\"\n",
       "@OutputFileName@ : name of the output file with default transliteration profile, e.g. \"|UTF8|IGNORE.txt\"\n",
       "@UnicodeEncoding@ : name of the character encoding scheme of unicode characters, e.g. \"UTF-8\", \"UTF-16BE\", \"UTF-32BE\", \"UTF-16LE\", or \"UTF-32LE\"\n",
       "@DefaultValue@ : the default value that will be the transliteration of all code points ;\n",
       "                 it must be an hexadecimal string of pair length, or one of the following values:",
       "                   -1 (ignore this character),\n",
       "                   -2, -3, ..., -128 (error on reading this character),\n",
       "                     this will be the error code returned so that you can freely dispatch characters into distinct error codes,\n",
       "Example : php fromUnicodeDataToDefaultProfile.php UnicodeData_10.0.0.txt \"|UTF8|ERROR.txt\" UTF-8 -2\n",
       "------------------------------------------------------------\n"
  ;

  die(1);
}



//Functions
function getSHexadecimalStringFrom($sLine, $sUnicodeEncoding){
  $arrFields = explode(';', $sLine);
  $sCodePoint = $arrFields[0];
  if(!ctype_xdigit($sCodePoint)){
    throw new Exception(sprintf('The code point "%s" is not hexadecimal.', $sCodePoint));
  }
  if(strlen($sCodePoint) > 6){
    throw new Exception(sprintf('The code point "%s" is too long.', $sCodePoint));
  }
  $iCodePoint = hexdec($sCodePoint);
  switch($sUnicodeEncoding){
    case 'UTF-8':
      if($iCodePoint <= 0x7F){ 
        return getSHexFromTinyInt( $iCodePoint ); 
      }
      if($iCodePoint <= 0x07FF){ 
        return getSHexFromTinyInt( intdiv($iCodePoint, 64) + 192 )
              .getSHexFromTinyInt( ($iCodePoint % 64) + 128 ); 
      }
      if($iCodePoint <= 0xFFFF){ 
        return getSHexFromTinyInt( intdiv($iCodePoint, 64 * 64) + 224 )
              .getSHexFromTinyInt( (intdiv($iCodePoint, 64) % 64) + 128 )
              .getSHexFromTinyInt( ($iCodePoint % 64) + 128 ); 
      }
      if($iCodePoint <= 0x10FFFF){ 
        return getSHexFromTinyInt( intdiv($iCodePoint, 64 * 64 * 64) + 240 )
              .getSHexFromTinyInt( (intdiv($iCodePoint, 64 * 64) % 64) + 128 )
              .getSHexFromTinyInt( (intdiv($iCodePoint, 64) % 64) + 128 )
              .getSHexFromTinyInt( ($iCodePoint % 64) + 128 ); 
      }
    throw new Exception(sprintf('The code point "%s" could not be converted to UTF-8.', $sCodePoint));

    case 'UTF-16BE':
      if($iCodePoint <= 0xFFFF){ 
        return getSHexFromTinyInt( intdiv($iCodePoint, 256) )
              .getSHexFromTinyInt( $iCodePoint % 256 ); 
      }
      if($iCodePoint <= 0x10FFFF){
        $iCodePoint -= 0x10000;
        $iHighSurrogate = intdiv($iCodePoint, 2**10) + 0xD800;
        $iLowSurrogate = ($iCodePoint % (2**10)) + 0xDC00;
        return getSHexFromTinyInt( intdiv($iHighSurrogate, 256) )
              .getSHexFromTinyInt( $iHighSurrogate % 256 )
              .getSHexFromTinyInt( intdiv($iLowSurrogate, 256) )
              .getSHexFromTinyInt( $iLowSurrogate % 256 ); 
      }
    throw new Exception(sprintf('The code point "%s" could not be converted to UTF-16BE.', $sCodePoint));
    
    case 'UTF-16LE':
      if($iCodePoint <= 0xFFFF){ 
        return getSHexFromTinyInt( $iCodePoint % 256 )
              .getSHexFromTinyInt( intdiv($iCodePoint, 256) ); 
      }
      if($iCodePoint <= 0x10FFFF){
        $iCodePoint -= 0x10000;
        $iHighSurrogate = intdiv($iCodePoint, 2**10) + 0xD800;
        $iLowSurrogate = ($iCodePoint % (2**10)) + 0xDC00;
        return getSHexFromTinyInt( $iHighSurrogate % 256 )
              .getSHexFromTinyInt( intdiv($iHighSurrogate, 256) )
              .getSHexFromTinyInt( $iLowSurrogate % 256 )
              .getSHexFromTinyInt( intdiv($iLowSurrogate, 256) ); 
      }
    throw new Exception(sprintf('The code point "%s" could not be converted to UTF-16LE.', $sCodePoint));

    case 'UTF-32BE':
      if($iCodePoint <= 0x10FFFF){
        return '00'//getSHexFromTinyInt( intdiv($iCodePoint, 2**24) )
              .getSHexFromTinyInt( intdiv($iCodePoint % (2**24), 2**16) )
              .getSHexFromTinyInt( intdiv($iCodePoint % (2**16), 2**8) )
              .getSHexFromTinyInt( $iCodePoint % 256 ); 
      }
    throw new Exception(sprintf('The code point "%s" could not be converted to UTF-32BE.', $sCodePoint));

    case 'UTF-32LE':
      if($iCodePoint <= 0x10FFFF){
        return getSHexFromTinyInt( $iCodePoint % 256 )
              .getSHexFromTinyInt( intdiv($iCodePoint % (2**16), 2**8) )
              .getSHexFromTinyInt( intdiv($iCodePoint % (2**24), 2**16) )
              .'00';//getSHexFromTinyInt( intdiv($iCodePoint, 2**24) )
      }
    throw new Exception(sprintf('The code point "%s" could not be converted to UTF-32LE.', $sCodePoint));
  }
}//end function getSHexadecimalStringFrom()



function getSHexFromTinyInt($i){
  if($i < 0){
    throw new Exception(sprintf('Forbidden negative tiny int %s.', $i));
  }
  if($i > 255){
    throw new Exception(sprintf('The tiny int %s is not tiny.', $i));
  }
  return str_pad(dechex($i), 2, '0', STR_PAD_LEFT);
}//end function getSHexFromTinyInt()



?>
