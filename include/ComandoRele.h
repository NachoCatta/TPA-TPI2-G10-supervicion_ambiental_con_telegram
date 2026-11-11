#ifndef COMANDO_RELE_H
#define COMANDO_RELE_H

// Valida el comando completo antes de convertir; el limite evita overflow.
inline bool parsearComandoRele(const char *texto, unsigned long maximo,
                               unsigned long &segundos) {
  const char prefijo[] = "/releon ";
  for (unsigned int i = 0; prefijo[i]; ++i) {
    if (*texto++ != prefijo[i]) return false;
  }
  while (*texto == ' ') ++texto;
  if (*texto < '0' || *texto > '9') return false;
  unsigned long valor = 0;
  while (*texto >= '0' && *texto <= '9') {
    unsigned long digito = static_cast<unsigned long>(*texto++ - '0');
    if (digito > maximo || valor > (maximo - digito) / 10UL) return false;
    valor = valor * 10UL + digito;
  }
  while (*texto == ' ') ++texto;
  if (*texto != '\0' || valor == 0) return false;
  segundos = valor;
  return true;
}

#endif
