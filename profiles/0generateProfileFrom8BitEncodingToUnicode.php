<?php

/*
This file is part of transliteration_profile_iconv library.

transliteration_profile_iconv is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

transliteration_profile_iconv is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with transliteration_profile_iconv.  If not, see <http://www.gnu.org/licenses/>.

©Copyright 2018 Laurent Lyaudet
*/

function getSHexFromTinyInt($i){
  if($i < 0){
    throw new Exception(sprintf('Forbidden negative tiny int %s.', $i));
  }
  if($i > 255){
    throw new Exception(sprintf('The tiny int %s is not tiny.', $i));
  }
  return str_pad(dechex($i), 2, '0', STR_PAD_LEFT);
}//end function getSHexFromTinyInt()



try{

  //check input
  if(!isset($argv[1])){
    throw new Exception('Please give the input encoding (arg 1).');
  }
  $sInputEncoding = $argv[1];

  if(!isset($argv[2])){
    throw new Exception('Please give the output encoding (arg 2).');
  }
  $sOutputEncoding = $argv[2];

  $s = '';
  for($i = 0; $i < 256; ++$i){

    $st = iconv($sInputEncoding, $sOutputEncoding, chr($i));

    $s .= getSHexFromTinyInt($i).' ';
    for($j = 0; $j < strlen($st); ++$j){
       $s .= getSHexFromTinyInt(ord($st[$j]));
    }
    $s .= "\n";
  }

  //echo $s;
  $sFileName = 'tp_'.$sInputEncoding.'__'.$sOutputEncoding.'.txt';
  file_put_contents($sFileName, $s);
}
catch(Exception $oException){
  echo $oException->getMessage(), "\n";

  echo "------------------------------------------------------------\n",
       "----- Script 0generateProfileFrom8BitEncodingToUnicode -----\n",
       "------------------------------------------------------------\n",
       "Usage : php 0generateProfileFrom8BitEncodingToUnicode.php @8BitEncoding@ @UnicodeEncoding@\n",
       "@8BitEncoding@ : name of the input encoding, e.g. \"ISO-8859-1\"\n",
       "@UnicodeEncoding@ : name of the output unicode encoding, e.g. \"UTF-8\"\n",
       "Example : php 0generateProfileFrom8BitEncodingToUnicode.php ISO-8859-1 UTF-8\n",
       "------------------------------------------------------------\n"
  ;

  die(1);
}
?>
