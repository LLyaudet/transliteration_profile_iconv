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
/*
This helper script echoes the modification to do in the tp_UTF-8__ISO-8859-1IGNORE.txt profile
to obtain the tp_benchmark.txt profile.
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



$arr = array(
  '€' => '¤',
  '➕' => '+',
  '’' => '\'',
  'ʼ' => '\'',
  'ʹ' => '\'',
  'ˈ' => '\'',
  '′' => '\'',
  '−' => '-',
  '➖' => '-',
  '‐' => '-',
  '‒' => '-',
  '–' => '-',
  '—' => '-',
  '―' => '-',
  ' ' => ' ',
);

foreach($arr as $sInput => $sOutput){
  $sOutput = iconv('UTF-8', 'ISO-8859-1//IGNORE', $sOutput);
  for($i = 0; $i < strlen($sInput); ++$i){
    echo getSHexFromTinyInt(ord($sInput[$i]));
  }
  echo " ";
  for($i = 0; $i < strlen($sOutput); ++$i){
    echo getSHexFromTinyInt(ord($sOutput[$i]));
  }
  echo "\n";
}

?>
