#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Cards are represented by the following bitmap:
// RGB123HSFPDQ. Valid cards have exactly one bit set per triplet.
// Valid sets have (for each triplet) a|b|c == 0b111 || a&b&c != 0;
class Card {
 public:
  int color;
  int number;
  int shape;
  int filling;

  inline bool operator==(const Card& rhs) {
    return
      color == rhs.color &&
      number == rhs.number &&
      shape == rhs.shape &&
      filling == rhs.filling;
  }
};

void print_card(Card* card) {
  if(card->number == 0) {
    printf("     ");
    return;
  }
  switch(card->color) {
    case 1: printf("\x1b[31m"); break;
    case 2: printf("\x1b[32m"); break;
    default: printf("\x1b[35m"); break;
  }
  char const * shape;
  const char *blank = " ";
  switch(card->shape) {
    case 1: switch (card->filling) {
      case 1: shape = "○"; break;
      case 2: shape = "◍"; break;
      default: shape = "●"; break;
    }; break;
    case 2: switch (card->filling) {
      case 1: shape = "▢"; break;
      case 2: shape = "▥"; break;
      default: shape = "▩"; break;
    }; break;
    default: switch (card->filling) {
      case 1: shape = "△"; break;
      case 2: shape = "◬"; break;
      default: shape = "▲"; break;
    }; break;
  }

  char picture[4];
  sprintf(picture, "%s%s%s",
    shape, 
    card->number > 1 ? shape : blank,
    card->number > 2 ? shape : blank);
  printf("(%s)", picture);
  printf("\x1b[0m");
}

bool is_set(Card*a, Card*b, Card*c) {
  return
    ((a->color & b->color & c->color) != 0 ||
       (a->color | b->color | c->color) == 7) &&
    ((a->number & b->number & c->number) != 0 ||
       (a->number | b->number | c->number) == 7) &&
    ((a->shape & b->shape & c->shape) != 0 ||
       (a->shape | b->shape | c->shape) == 7) &&
    ((a->filling & b->filling & c->filling) != 0 ||
       (a->filling | b->filling | c->filling) == 7);
}

void print_deck(Card *cards,
                int cards_out,
                int cards_left,
                bool print_cards = true) {
  printf("Table:\n");
  for(int i = 0; i < cards_out/3; i++) {
    for(int j = 0; j < 3; j++) {
      print_card(cards + i*3 + j);
    }
    printf("\n");
  }
  if(!print_cards)
    return;
  printf("-------------\n");
  printf("Deck: ");
  for(int i = cards_out; i < cards_left; i++) {
    print_card(cards + i);
  }
  printf("\n");
  printf("=============\n");
}

int count_sets(Card* cards, int cards_out) {
  int total = 0; 
  for(int i = 0; i < cards_out-2; i++) {
    for(int j = i+1; j < cards_out-1; j++) {
      for(int k = j+1; k < cards_out; k++) {
        if(is_set(cards+i, cards+j, cards+k)) {
          printf("Found set: ");
          print_card(cards+i);
          print_card(cards+j);
          print_card(cards+k);
          printf("\n");
          total++;
        }
      }
    }
  }
  return total;
}


bool find_sets(Card* cards, int cards_out, int *cards_left) {
  if(cards_out > 15) {
    printf("Lots of cards out.\n");
  }
  print_deck(cards, cards_out, *cards_left);
  printf("Found %d sets\n", count_sets(cards, cards_out));
  for(int i = 0; i < cards_out-2; i++) {
    for(int j = i+1; j < cards_out-1; j++) {
      for(int k = j+1; k < cards_out; k++) {
        if(is_set(cards+i, cards+j, cards+k)) {
          printf("Retrieving set:");
          print_card(cards+i);
          print_card(cards+j);
          print_card(cards+k);
          printf("\n-=-=-=\n");
          cards[i].number = 0;
          cards[j].number = 0;
          cards[k].number = 0;
          print_deck(cards, cards_out, *cards_left, false);
          printf("\n=-=-=-\n");
          // Do we have any cards left?
          if(*cards_left > cards_out && cards_out < 15) {
            cards[i] = cards[*cards_left-1];
            cards[j] = cards[*cards_left-2];
            cards[k] = cards[*cards_left-3];
          } else {
            // otherwise, just move all cards down.
            for(int x = 0; x < *cards_left; x++) {
              if(cards[x].number == 0) {
                // find the next non-blank and swap.
                for(int y = x; y < *cards_left; y++) {
                  if(cards[y].number != 0) {
                    cards[x] = cards[y];
                    cards[y].number = 0;
                    break;
                  }
                }
              }
            }
          }
          *cards_left -= 3;
          return true;
        }
      }
    }
  }
  // We couldn't find cards.
  if(cards_out < *cards_left) {
    printf("No set.\n");
    return find_sets(cards, cards_out+3, cards_left);
  }
  return false;
}

void shuffle_deck(Card *deck) {
  for(int i = 0; i < 81; i++) {
    // pick a random card to shuffle with.
    int j = rand() % 81;
    Card tmp = deck[j];
    deck[j] = deck[i];
    deck[i] = tmp;
  }
}

void fill_deck(Card *deck) {
  for(int i = 0; i < 81; i++) {
    deck[i].color = 1 << (i % 3);
    deck[i].number = 1 << (i / 3 % 3);
    deck[i].shape = 1 << (i / 9 % 3);
    deck[i].filling = 1 << (i / 27 % 3);
  }
}

void verify_deck(Card *deck) {
  for(int i = 0; i < 80; i++) {
    for(int j = i+1; j < 81; j++) {
      if(deck[i] == deck[j]) {
        printf("Deck has dupes!\n");
      }
    }
  }
}

int main() {
  srand(time(NULL));
  Card deck[81];
  fill_deck(deck);

#if 0
  int counts[10];
#define ITERATIONS 10000
  for(int i = 0; i < ITERATIONS; i++ ) {
    shuffle_deck(deck);
    int count = count_sets(deck, 81);
    counts[count] ++;
  }
  for(int j = 0; j<10; j++) {
    float percent = counts[j];
    percent =percent/ ITERATIONS * 100;
    printf("(%d:%2.2f)", j, percent);
  }
  printf("\n");
#endif

#define ITERATIONS 10000
  int counts[7];
  for(int i = 0; i < ITERATIONS; i++) {
    fill_deck(deck);
    shuffle_deck(deck);
    verify_deck(deck);
    int cards_left = 81;
    while(find_sets(deck, cards_left < 12 ? cards_left : 12, &cards_left)) {
    }
    // Ran out of cards.
    printf("Remaining cards: [%d]\n", cards_left);
    counts[cards_left/3] ++;
  }
  printf("\n\n");
  for(int j = 0; j < 7; j++) {
    float percent = counts[j];
    percent =percent/ ITERATIONS * 100;
    printf("(%d:%2.2f)\n", j*3, percent);
  }
  printf("\n\n");
}
