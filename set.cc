#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <ncurses.h>

// Card represents one card in the Set deck.
// Attributes are stored as integers, either 1, 2, or 4.
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
    printw("     ");
    return;
  }
  attron(COLOR_PAIR(card->color));

  char const *shape;
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

  printw("(%s%s%s)",
         shape, 
         card->number > 1 ? shape : " ",
         card->number > 2 ? shape : " ");
  attroff(COLOR_PAIR(card->color));
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
//  clear();
  printw("Table:\n");
  for(int i = 0; i < cards_out/3; i++) {
    for(int j = 0; j < 3; j++) {
      print_card(cards + i*3 + j);
    }
    printw("\n");
  }
  if(!print_cards)
    return;
  printw("-------------\n");
  printw("Deck: ");
  for(int i = cards_out; i < cards_left; i++) {
    print_card(cards + i);
  }
  printw("\n");
  printw("=============\n");
  refresh();
}

int count_sets(Card* cards, int cards_out) {
  int total = 0; 
  for(int i = 0; i < cards_out-2; i++) {
    for(int j = i+1; j < cards_out-1; j++) {
      for(int k = j+1; k < cards_out; k++) {
        if(is_set(cards+i, cards+j, cards+k)) {
          printw("Found set: ");
          print_card(cards+i);
          print_card(cards+j);
          print_card(cards+k);
          printw("\n");
          total++;
        }
      }
    }
  }
  return total;
}


bool find_sets(Card* cards, int cards_out, int *cards_left) {
  if(cards_out > 15) {
    printw("Lots of cards out.\n");
  }
  print_deck(cards, cards_out, *cards_left);
  printw("Found %d sets\n", count_sets(cards, cards_out));
  for(int i = 0; i < cards_out-2; i++) {
    for(int j = i+1; j < cards_out-1; j++) {
      for(int k = j+1; k < cards_out; k++) {
        if(is_set(cards+i, cards+j, cards+k)) {
          printw("Retrieving set:");
          print_card(cards+i);
          print_card(cards+j);
          print_card(cards+k);
          printw("\n-=-=-=\n");
          cards[i].number = 0;
          cards[j].number = 0;
          cards[k].number = 0;
          print_deck(cards, cards_out, *cards_left, false);
          printw("\n=-=-=-\n");
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
    printw("No set.\n");
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
        printw("Deck has dupes!\n");
      }
    }
  }
}

void init_display() {
  // Get ncurses started.
  initscr();
  start_color();
  init_pair(1, COLOR_RED, COLOR_WHITE);
  init_pair(2, COLOR_GREEN, COLOR_WHITE);
  init_pair(4, COLOR_MAGENTA, COLOR_WHITE);
}

void finalize_display() {
  refresh();
  endwin();
}

int main(int argc, char **argv) {
  // We use random numbers for shuffling.
  srand(time(NULL));

  Card deck[81];
  fill_deck(deck);
  
  init_display();

  // Parse command-line parameters.
  int iterations = 0;
  if(argc != 2 || sscanf(argv[1], "%d", &iterations) != 1) {
    printw("Usage: %s ITERATIONS\n", argv[0]);
    finalize_display();
    exit(1);
  }

  int counts[7];
  for(int i = 0; i < iterations; i++) {
    fill_deck(deck);
    shuffle_deck(deck);
    verify_deck(deck);
    int cards_left = 81;
    while(find_sets(deck, cards_left < 12 ? cards_left : 12, &cards_left)) {
    }
    // Ran out of cards.
    printw("Remaining cards: [%d]\n", cards_left);
    counts[cards_left/3] ++;
  }
  printw("\n\n");
  for(int j = 0; j < 7; j++) {
    float percent = counts[j];
    percent =percent/ iterations * 100;
    printw("(%d:%2.2f)\n", j*3, percent);
  }
  printw("\n\n");

  finalize_display();
}
