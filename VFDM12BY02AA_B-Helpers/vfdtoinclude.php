<?php
	$f=file("asciivfd.txt");
	$fpOut=fopen("vfdascii.h","w");
	fwrite($fpOut,"unsigned char  vfdchars[] PROGMEM={\n");
	for ($i=0; $i < count($f); $i++)
	{
		$line=$f[$i];
		if (preg_match("/(.) ([-abcdefghjkmnps12r]+)/",$line,$m))
		{
			//print_r($m);
			$v=0;
			$w=0;
			$c=$m[1];
			for ($j=0; $j < strlen($m[2]);$j++)
			{
				switch($m[2][$j])
				{
					case 'a': $v|=1; break;
					case 'b': $v|=2; break;
					case 'c': $v|=4; break;
					case 'd': $v|=8; break;
					case 'e': $v|=16; break;
					case 'f': $v|=32; break;
					case 'g': $v|=64; break;
					case 'h': $v|=128; break;
					case 'j': $w|=1; break; 
					case 'k': $w|=2; break; 
					case 'm': $w|=4; break; 
					case 'n': $w|=8; break; 
					case 'p': $w|=16; break;
					case 's': 
						$j++;
						if ($m[2][$j]=='1')
							$w|=64;
						if ($m[2][$j]=='2')
							$w|=128;
						break;
					case 'r': $w|=32; break;
				}
			}
			fwrite($fpOut,"$v,$w, //$i ' $c '\n");
		}
		else
		{
			fwrite($fpOut, "0, 0, //line $i doesn't match pattern ($line)\n");
			echo "line $i doesn't match pattern ($line)\n";
		}
	}
	fwrite($fpOut, "};\n");

?>